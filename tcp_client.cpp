//
//  tcp_client.cpp
//  go-socket-client
//
//  Copyright © 2023 Yan. All rights reserved.
//

#include "tcp_client.hpp"
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include "socket/socket_select.hpp"
#include <stdint.h>
#include "common/alarm.hpp"
#include <iostream>
#include "log/zlogger.hpp"
#include <errno.h>
#include "common/time_utils.h"
#include <regex>
#include <netdb.h>
#include "common/scope_guard.hpp"
#include "connect/complex_connect.hpp"
#include "connect/dns_query.hpp"

const int kDnsTimeout = 2; //DNS超时时间，秒

const int kConnectTimeout = 10 * 1000; //复合连接超时，毫秒
const int kConnectInterval = 4 * 1000; //复合连接间隔，毫秒
const int kConnectErrorInterval = 0; //复合连接出错间隔，毫秒
const int kMaxConnectNum = 3; //复合连接最大并发数量

const int kSocketTimeout = 5; //socket的读取和写入超时，秒
const int kPingInterval = 60; //心跳间隔，秒
const int kPingTimeout = 5; //心跳超时，秒
const int kSendReqTimeout = 10; //调用接口超时，秒

const int8_t kRetConnFailed = 0; //连接失败
const int8_t kRetConnSuccess = 1; //连接成功
const int8_t kRetConnTimeout = -1; //ConnAck包超时，用于判断是否禁用linger
const int8_t kRetConnKickout = -2; //Token错误，一般是被踢出登陆

TcpClient::TcpClient()
    : _disCode(kDisCodeNone)
    , _connectStatus(kConnectIdle)
    , _thread(NULL)
    , _reconnectCount(0)
    , _currentMsgId(1)
    , _lastConnTick(0)
    , _socket(NULL)
    , _netType(kNetTypeUnknown)
{
    _disCodeMap = {
        {kDisCodeNone, "none"},
        {kDisCodeReset, "reset"},
        {kDisCodeNetChange, "net change"},
        {kDisCodeNetLost, "net lost"},
        {kDisCodeKickout, "kick out"},
        {kDisCodeLogout, "logout"},
        {kDisCodeConnectTimeout, "connect timeout"},
        {kDisCodeSendReqTimeout, "send req timeout"},
        {kDisCodePingTimeout, "ping timeout"},
    };
}

TcpClient::~TcpClient() {
    Disconnect(kDisCodeReset, false);
}

void TcpClient::InitHostAndPort(std::string host, std::string backupIp, int port, bool isTls, GetConnectInfo getConnectInfo) {
    _host = host;
    _backupIp = backupIp;
    _port = port;
#ifdef WITH_SSL
    _isTls = isTls;
#else
    _isTls = false;
#endif
    _getConnectInfo = getConnectInfo;
    _reconnectAlarm.SetCallback([this](){
        zcdebug2("reconnect callback");
        //确保线程已退出
        this->__WaitDisconnect();
        //如果需要自动重连且重连次数未超过限制
        if (_shouldReconnect) {
            _reconnectCount++;
            this->MakeSureConnected();
        }
    });
    _pingAlarm.SetCallback([this](){
        //发信号通知线程
        this->_readWriteBreaker.Break();
    });
    _pingTimeoutAlarm.SetCallback([this](){
        //发信号通知线程
        this->_readWriteBreaker.Break();
    });
}

void TcpClient::SetHttpDns(GetHttpDns getHttpDns) {
    DnsQuery::SharedQuery()->SetHttpDns(getHttpDns);
}

void TcpClient::MakeSureConnected() {
    std::lock_guard<std::mutex> lock(_mutex);
    if (_connectStatus == kConnected) {
        zcdebug2("already connected");
        return;
    }
    if (_thread == NULL) {
        zcdebug2("new thread");
        //重置连接状态
        _connectStatus = kConnectIdle;
        //重置断开连接原因
        _disCode = kDisCodeNone;
        //默认自动重连
        _shouldReconnect = true;
        //默认不用重建breaker
        _shouldNewBreaker = false;
        _shouldNewConnBreaker = false;
        //超时断开连接
        _timeoutDisconnect = false;
        //清空breaker
        _readWriteBreaker.Clear();
        _connectBreaker.Clear();
        //清空消息队列
        while (!_messageQueue.empty()) {
            _messageQueue.pop();
        }
        _thread = new std::thread(&TcpClient::__Run, this);
    }else {
        zcdebug2("nothing");
    }
}

void TcpClient::DisconnectAsync(DisconnectCode code, bool shouldReconnect) {
    zcdebug2("TcpClient::DisconnectAsync");
    std::unique_lock<std::mutex> lock(_mutex);
    //如果线程没有运行
    if (_thread == NULL) {
        //如果有重连标志
        if (shouldReconnect) {
            //连接之前必须解锁
            lock.unlock();
            this->MakeSureConnected();
        }
        return;
    }
    //记录断开连接的方式
    _disCode = code;
    //自动重连
    _shouldReconnect = shouldReconnect;
    //发信号
    if (!_readWriteBreaker.Break()) {
        zcwarn2("read write breaker fail");
        _readWriteBreaker.Close();
        _shouldNewBreaker = true;
    }
    if (!_connectBreaker.Break()) {
        zcwarn2("connect breaker fail");
        _connectBreaker.Close();
        _shouldNewConnBreaker = true;
    }
}

void TcpClient::Disconnect(DisconnectCode code, bool shouldReconnect) {
    this->DisconnectAsync(code, shouldReconnect);
    this->__WaitDisconnect();
}

void TcpClient::__WaitDisconnect() {
    if (_thread == NULL) return;
    //阻塞到线程结束
    _thread->join();
    delete _thread;
    _thread = NULL;
    //停止所有超时计时
    std::vector<int> timeoutMessageIds;
    std::unique_lock<std::mutex> lock(_timeoutMutex);
    for (auto iter = _timeoutAlarmMap.begin(); iter != _timeoutAlarmMap.end(); iter++) {
        timeoutMessageIds.push_back(iter->first);
        auto alarmPtr = std::move(iter->second);
        //取消超时计时
        alarmPtr->Cancel();
    }
    _timeoutAlarmMap.clear();
    lock.unlock();
    //调用所有回调
    for (auto messageId : timeoutMessageIds) {
        OnSendRespCallback callback = this->__GetCallback(messageId);
        if (callback == NULL) return;
        //异步调用，避免卡线程
        std::thread asyncCall([callback](){
            callback(kErrorCodeLoseConnect, "");
        });
        asyncCall.detach();
    }
    //如果管道出错，重建
    if (_shouldNewBreaker) {
        _readWriteBreaker.ReCreate();
    }
    if (_shouldNewConnBreaker) {
        _connectBreaker.ReCreate();
    }
}

bool TcpClient::Send(std::string payloadType, std::string payload, OnSendRespCallback func) {
    return Send(payloadType, payload, NULL, 0, func);
}

bool TcpClient::Send(std::string payloadType, std::string payload, char *data, int dataLen, OnSendRespCallback func) {
    //注意，不要出现持有其它锁之后再请求_mutex锁的情况，否则会产生死锁
    std::lock_guard<std::mutex> lock(_mutex);
    if (_connectStatus != kConnected) {
        if (func != NULL) {
            //必须异步，否则会死锁
            std::thread asyncCall([func]() {
                func(kErrorCodeNotConnect, "");
            });
            asyncCall.detach();
        }
        return false;
    }
    //记录回调
    std::unique_lock<std::mutex> mapLock(_mapMutex);
    _callbackMap[_currentMsgId] = func;
    mapLock.unlock();
    //构造消息
    SendReq *sendReq = new SendReq();
    sendReq->SetType(payloadType);
    sendReq->SetPayload(payload);
    sendReq->SetMessageId(_currentMsgId);
    if (data != NULL) {
        sendReq->SetData(data, dataLen);
    }
    _messageQueue.push(std::unique_ptr<MessageBase>(sendReq));
    //维护消息id
    _currentMsgId++;
    if (_currentMsgId == UINT16_MAX) {
        _currentMsgId = 1;
    }
    //发信号通知线程
    _readWriteBreaker.Break();
    return true;
}

bool TcpClient::SendNoReply(std::string type, std::string payload) {
    //注意，不要出现持有其它锁之后再请求_mutex锁的情况，否则会产生死锁
    std::lock_guard<std::mutex> lock(_mutex);
    if (_connectStatus != kConnected) {
        return false;
    }
    //构造消息
    SendReq *sendReq = new SendReq();
    sendReq->SetType(type);
    sendReq->SetPayload(payload);
    sendReq->SetReplyLevel(SendReq::RLevelNoReply);
    sendReq->SetMessageId(0);
    _messageQueue.push(std::unique_ptr<MessageBase>(sendReq));
    //发信号通知线程
    _readWriteBreaker.Break();
    return true;
}

void TcpClient::SendDisconnect() {
    std::lock_guard<std::mutex> lock(_mutex);
    if (_connectStatus != kConnected) return;
    //构造断开连接消息
    class Disconnect *disconnect = new class Disconnect();
    _messageQueue.push(std::unique_ptr<MessageBase>(disconnect));
    //发信号通知线程
    _readWriteBreaker.Break();
}

void TcpClient::__Run() {
    zcdebug2("pre run connect");
    int8_t ret = __RunConnect();
    //修改连接状态
    ConnectStatus status = kConnectFailed;
    if (ret == kRetConnSuccess) {
        status = kConnected;
    }else if (ret == kRetConnKickout) {
        status = kConnectKickout;
    }else {
        status = kConnectFailed;
    }
    __SetConnectStatus(status);
    //如果连接成功
    if (ret == kRetConnSuccess) {
        zcdebug2("start readwrite");
        //进入读写循环
        __RunReadWrite();
        zcdebug2("end readwrite");
    }
    //如果是连接超时，需要在关闭socket之前禁用Linger Time，实现立即关闭socket
    //否则客户端未发出的Connect包可能在关闭Socket之后的TIME_WAIT状态下发出（弱网状况）
    //导致服务器收到一个无用的Connect包
    else if (ret == kRetConnTimeout) {
        struct linger so_linger;
        so_linger.l_onoff = 1;
        so_linger.l_linger = 0;
        //连接即将断开，故忽略返回结果，但如果失败有可能导致上面说的结果
        SOCKET socketFd = _socket->GetSocket();
        int setRet = setsockopt(socketFd, SOL_SOCKET, SO_LINGER, &so_linger, sizeof(so_linger));
        if (setRet != 0) {
            zcerror2("setsockopt linger to 0 error");
        }
    }
    //循环退出
    //关闭socket
    if (_socket != NULL) {
        _socket->Close();
        delete _socket;
        _socket = NULL;
    }
    //修改连接状态为断开连接
    __SetConnectStatus(kDisconnected);
    //循环退出后清空资源
    _reconnectAlarm.Start(0);
}

int8_t TcpClient::__RunConnect() {
    __SetConnectStatus(kConnecting);

    uint64_t currentTick = gettickcount();
    uint64_t lastConnDuration = currentTick - _lastConnTick;
    //如果两次连接间隔小于5分钟，或者第一次
    if (lastConnDuration < 300000 || _lastConnTick == 0) {
        //如果重连超过一定次数，sleep1秒再连接，避免短时间内大量重连
        if (_reconnectCount > 5) {
            zcinfo2("reconnect count:%d, sleep 1 second", _reconnectCount);
            std::this_thread::sleep_for(std::chrono::seconds(1));
            zcinfo2("start connecting");
        }else {
            zcinfo2("start connecting immediately, count:%d", _reconnectCount);
        }
    }else {
        //5分钟以上未连接，重置连接次数，避免用户有网之后还要再等几秒
        _reconnectCount = 0;
        zcinfo2("start connecting immediately after long time");
    }
    //如果没网
    if (_netType == kNetTypeNoNet) {
        zcinfo2("no net, connect failed");
        //不自动重连
        _shouldReconnect = false;
        return kRetConnFailed;
    }
    //没有host
    if (_host == "") {
        zcwarn2("empty host");
        return kRetConnFailed;
    }
    //没有备用ip
    if (_backupIp == "") {
        zcwarn2("empty backup ip");
        return kRetConnFailed;
    }

    //解析域名
    std::vector<std::string> ips;
    bool dnsRet = DnsQuery::SharedQuery()->GetHostByName(_host, _backupIp, ips, _connectBreaker, kDnsTimeout);
    //如果出错
    if (!dnsRet) {
        return kRetConnFailed;
    }
    //生成复合连接的所有地址
    std::vector<AddrInfo> addrs;
    int ipIndex = 0;
    //确保地址数量到达3，这样即使只有一个ip，也可以复合开多个连接确保连接速度
    while (addrs.size() < 3) {
        addrs.push_back(AddrInfo{ips[ipIndex], _port, _isTls});
        ipIndex++;
        if (ipIndex >= ips.size()) {
            ipIndex = 0;
        }
    }
    //开始连接
    ComplexConnect complexConn(kConnectTimeout, kConnectInterval, kConnectErrorInterval, kMaxConnectNum);
    _socket = complexConn.ConnectImpatient(addrs, _connectBreaker);
    if (_socket == NULL || _socket->IsInvalid()) {
        zcerror2("invalid socket");
        return kRetConnFailed;
    }
    SOCKET socketFd = _socket->GetSocket();
    //设置写入和读取的超时时间，需要在变为阻塞模式之后设置
    struct timeval timeout;
    timeout.tv_sec = kSocketTimeout;
    timeout.tv_usec = 0;
    int ret = setsockopt(socketFd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));
    if (ret != 0) {
        zcerror2("setsockopt receive timeout error");
        return kRetConnFailed;
    }
    ret = setsockopt(socketFd, SOL_SOCKET, SO_SNDTIMEO, &timeout, sizeof(timeout));
    if (ret != 0) {
        zcerror2("setsockopt send timeout error");
        return kRetConnFailed;
    }
    _writer.SetSocket(_socket);
    _reader.SetSocket(_socket);
    //wifi网络下修改MSS，部分地区网络需要MSS1400才能发包
    if (_netType == kNetTypeWifi && socket_fix_tcp_mss(socketFd) < 0) {
#ifdef ANDROID
        zcinfo2("wifi set tcp mss error:%0", strerror(errno));
#endif
    }
    //禁用nagle算法，加快TCP速度
    if (0 != socket_disable_nagle(socketFd, 1)) {
        zcerror2("socket_disable_nagle sock:%0, %1(%2)", socketFd, errno, strerror(errno));
    }
    //获取连接信息
    if (_getConnectInfo == NULL) {
        zcwarn2("empty connect info");
        return kRetConnFailed;
    }
    uint64_t lastTick = gettickcount();
    std::string connectInfo = _getConnectInfo();
    //发送连接包
    class Connect connect;
    connect.SetPayload(connectInfo);
    connect.SetProtocolName(kProtocolName);
    connect.SetProtocolVersion(kProtocolVersion);
    connect.SetKeepAlive(kPingInterval);
    connect.SetEnableGzip(kEnableGzip);
    try {
        _msgManager.EncodeMessage(&_writer, &connect);
    } catch (TcpException &e) {
        zcerror2("send connect request error:%s", e.What().c_str());
        return kRetConnFailed;
    }
    //接收返回包
    uint8_t returnRet = kRetConnFailed;
    try {
        ConnAck *connAck = (ConnAck *)_msgManager.DecodeMessage(&_reader).get();
        ConnAck::ReturnCode code = connAck->GetReturnCode();
        if (code == ConnAck::RetCodeAccepted) {
            returnRet = kRetConnSuccess;
        }else {
            zcerror2("connack return code:%d", connAck->GetReturnCode());
            if (code == ConnAck::RetCodeNotAuthorized) {
                returnRet = kRetConnKickout;
                //如果被踢出登陆，不再重连
                _shouldReconnect = false;
                zcwarn2("connect kick out error");
            }
        }
    } catch (TcpException &e) {
        zcerror2("receive connect response error:%s", e.What().c_str());
        if (e.GetErrno() == EAGAIN) {
            return kRetConnTimeout;
        }else {
            return kRetConnFailed;
        }
    }
    //记录登陆时间
    uint64_t finLoginTime = gettickcount();
    zcinfo2("finish login:%d ms", finLoginTime-lastTick);
    return returnRet;
}

void TcpClient::__RunReadWrite() {
    //函数结束的时候停止心跳定时器
    ScopeGuard scopeGuard([this](){
        this->_pingAlarm.Cancel();
        this->_pingTimeoutAlarm.Cancel();
    });
    uint64_t loopStartTick = gettickcount();
    while (1) {
        uint64_t curTick = gettickcount();
        //如果连接稳定超过5秒，重置重连次数
        if (_reconnectCount != 0 && (curTick-loopStartTick) > 5*1000) {
            _reconnectCount = 0;
        }
        //如果心跳计时未开始
        if (!_pingAlarm.IsWaiting()) {
            _pingAlarm.Start(kPingInterval);
        }
        SocketSelect sel(_readWriteBreaker);
        sel.PreSelect();
        SOCKET socketFd = _socket->GetSocket();
        //监听端口是否可读
        sel.ReadFdSet(socketFd);
        //监听端口是否报错
        sel.ExceptionFdSet(socketFd);
        //如果有待发送的消息，监听端口是否可写
        std::unique_lock<std::mutex> lock(_mutex);
        bool hasMessage = !_messageQueue.empty();
        lock.unlock();
        if (hasMessage) {
            sel.WriteFdSet(socketFd);
        }
        int selRet = sel.Select();
        //如果断开连接
        if (_disCode != kDisCodeNone) {
            zcwarn2("socket close sock:%d, user disconnect:%s, nread:%d, nwrite:%d", socketFd, _disCodeMap[_disCode].c_str(), socket_nread(socketFd), socket_nwrite(socketFd));
            return;
        }
        //监听出错
        if (selRet < 0) {
            zcfatal2("socket close sock:%d, selRet < 0, errno:%d, nread:%d, nwrite:%d", socketFd, sel.GetErrno(), socket_nread(socketFd), socket_nwrite(socketFd));
            return;
        }
        //breaker出错
        if (sel.IsException()) {
            zcerror2("socket close sock:%d, socketselect exception:%d(%s), nread:%d, nwrite:%d", socketFd, errno, strerror(errno), socket_nread(socketFd), socket_nwrite(socketFd));
            return;
        }
        //端口出错
        if (sel.ExceptionFdIsSet(socketFd)) {
            zcerror2("socket close sock:%d, exception:%d(%s), nread:%d, nwrite:%d", socketFd, errno, strerror(errno), socket_nread(socketFd), socket_nwrite(socketFd));
            int status = 0;
            socklen_t len = sizeof(status);
            int ret = getsockopt(socketFd, SOL_SOCKET, SO_ERROR, &status, &len);
            if (ret != 0) {
                zcerror2("sock:%d, get socketopt error", socketFd);
            }else {
                zcerror2("sock:%d, fd error:%d, str:%s", socketFd, status, strerror(status));
            }
            return;
        }
        //心跳超时
        if (_pingTimeoutAlarm.IsAlarm()) {
            _disCode = kDisCodePingTimeout;
            zcerror2("socket close sock:%d, ping timeout, nread:%d, nwrite:%d", socketFd, socket_nread(socketFd), socket_nwrite(socketFd));
            return;
        }
        //加锁
        lock.lock();
        hasMessage = !_messageQueue.empty();
        lock.unlock();
        //端口可写且有待写入的消息
        if (sel.WriteFdIsSet(socketFd) && hasMessage) {
            //可写的时候只写一条消息，避免网络堵塞
            lock.lock();
            MessageBase *message = _messageQueue.front().get();
            lock.unlock();
            try {
                _msgManager.EncodeMessage(&_writer, message);
            } catch (TcpException &e) {
                zcerror2("send message error:%s", e.What().c_str());
                return;
            }
            //如果是SendReq消息
            if (dynamic_cast<SendReq*>(message) != nullptr) {
                SendReq *sendReqMsg = (SendReq *)message;
                //设置超时
                auto alarmPtr = _objectPool.Get();
                Alarm *alarm = alarmPtr.get();
                alarm->SetUserData(sendReqMsg->GetMessageId());
                alarm->SetCallback([alarm, this](){
                    int messageId = alarm->GetUserData();
                    //移除计时器
                    std::unique_lock<std::mutex> timeoutLock(_timeoutMutex);
                    auto iter = _timeoutAlarmMap.find(messageId);
                    if (iter != _timeoutAlarmMap.end()) {
                        _timeoutAlarmMap.erase(messageId);
                    }
                    timeoutLock.unlock();
                    //调用回调
                    OnSendRespCallback callback = this->__GetCallback(messageId);
                    if (callback == NULL) return;
                    //异步调用，避免卡线程
                    std::thread asyncCall([callback](){
                        callback(kErrorCodeTimeout, "");
                    });
                    asyncCall.detach();
                    //如果在连接中
                    if (this->_connectStatus == kConnected && !_timeoutDisconnect) {
                        //确保超时断开连接只调用一次
                        _timeoutDisconnect = true;
                        this->DisconnectAsync(kDisCodeSendReqTimeout, true);
                    }else {
                        zcdebug2("timeout without reconnect status: %d, has reconnect:%d", this->_connectStatus, _timeoutDisconnect);
                    }
                });
                alarm->Start(kSendReqTimeout);
                //记录计时器
                _timeoutMutex.lock();
                _timeoutAlarmMap[sendReqMsg->GetMessageId()] = std::move(alarmPtr);
                _timeoutMutex.unlock();
            }
            //移除消息
            lock.lock();
            _messageQueue.pop();
            lock.unlock();
            //刚写入了数据，重置心跳计时
            _pingAlarm.Cancel();
            _pingAlarm.Start(kPingInterval);
        }

        //端口可读
        if (sel.ReadFdIsSet(socketFd)) {
            MessageBase *message = NULL;
            try {
                //这里必须用release，否则message离开当前作用域就会被释放
                message = _msgManager.DecodeMessage(&_reader).release();
            }catch (TcpException& e) {
                zcerror2("receive message error:%s", e.What().c_str());
                return;
            }
            switch (message->GetMsgType()) {
                case kMsgPingResp:
                    //停止心跳包超时计时
                    _pingTimeoutAlarm.Cancel();
                    break;
                case kMsgSendReq:
                    __handleSendReq((SendReq *)message);
                    break;
                case kMsgSendResp:
                    __handleSendResp((SendResp *)message);
                    break;
                case kMsgDisconnect:
                    __handleDisconnect((class Disconnect *)message);
                    break;
                default:
                    break;
            }
            //之前用了release，这里必须手动释放
            delete message;
        }
        //心跳计时到期
        if (_pingAlarm.IsAlarm()) {
            //发送心跳包
            try {
                PingReq *pingReq = new PingReq();
                _msgManager.EncodeMessage(&_writer, pingReq);
                delete pingReq;
                zcdebug2("send ping");
            } catch (TcpException &e) {
                zcerror2("send ping error:%s", e.What().c_str());
                return;
            }
            //启动心跳包超时计时
            _pingTimeoutAlarm.Cancel();
            _pingTimeoutAlarm.Start(kPingTimeout);
        }
    }
}

void TcpClient::__SetConnectStatus(ConnectStatus status) {
    if (status == _connectStatus) return;
    _connectStatus = status;
    if (_onStatusChange != NULL) {
        _onStatusChange(status);
    }
}

OnSendRespCallback TcpClient::__GetCallback(uint16_t messageId) {
    //加锁
    std::unique_lock<std::mutex> lock(_mapMutex);
    auto iter = _callbackMap.find(messageId);
    OnSendRespCallback callback = NULL;
    //如果有相应的回调
    if (iter != _callbackMap.end()) {
        callback = _callbackMap[messageId];
        //删除回调
        _callbackMap.erase(messageId);
    }
    //执行回调之前解锁，避免长时间占有锁
    return callback;
}

void TcpClient::__handleSendReq(SendReq *sendReq) {
    if (_onSendReq != NULL) {
        std::string msgType = sendReq->GetType();
        std::string msgPayload = sendReq->GetPayload();
        //异步调用，避免卡线程
        std::thread asyncCall([this, msgType, msgPayload]() {
            this->_onSendReq(msgType, msgPayload);
        });
        asyncCall.detach();
    }
}

void TcpClient::__handleSendResp(SendResp *sendResp) {
    uint16_t messageId = sendResp->GetMessageId();
    //获取超时计时器
    std::unique_lock<std::mutex> timeoutLock(_timeoutMutex);
    auto iter = _timeoutAlarmMap.find(messageId);
    if (iter != _timeoutAlarmMap.end()) {
        auto alarmPtr = std::move(_timeoutAlarmMap[messageId]);
        //取消超时计时
        alarmPtr->Cancel();
        _timeoutAlarmMap.erase(messageId);
    }
    timeoutLock.unlock();
    //获取回调
    OnSendRespCallback callback = __GetCallback(messageId);
    //执行回调
    if (callback != NULL) {
        std::string payload = sendResp->GetPayload();
        //异步调用，避免卡后面的请求
        std::thread asyncCall([callback, payload]() {
            callback(kErrorCodeNone, payload);
        });
        asyncCall.detach();
    }
}

void TcpClient::__handleDisconnect(class Disconnect *disconnect) {
    SvrDisCode disCode = (SvrDisCode)disconnect->GetDisconnectType();
    zcinfo2("disconnect received: %d", disCode);
    //Android的退出登录时间会延时调用，所以这里必须断开连接
    //但同时导致iOS和android都会触发两次DisconnectAsync，可以忽略
    if (disCode == SvrDisKickout) {
        DisconnectAsync(kDisCodeKickout, false);
    }
    if (_onServerDisconnect != NULL) {
        _onServerDisconnect(disCode);
    }
}
