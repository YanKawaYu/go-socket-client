//
//  tcp_client.hpp
//  go-socket-client
//
//  Copyright © 2023 Yan. All rights reserved.
//

#ifndef tcp_client_hpp
#define tcp_client_hpp

#include <stdio.h>
#include "stdint.h"
#include "packet/unix_socket.hpp"
#include "packet/message.hpp"
#include "packet/message_manager.hpp"
#include "socket/socket_breaker.hpp"
#include <map>
#include <queue>
#include "common/alarm.hpp"
#include "object_pool.hpp"
#include <memory>
#include <thread>
#include <mutex>
#include "tcp_define.h"
#include "socket/tcp_socket.hpp"

class TcpClient {
public:
    TcpClient();
    ~TcpClient();
    void InitHostAndPort(std::string host, std::string backupIp, int port, bool isTls, GetConnectInfo getConnectInfo);
    void MakeSureConnected();
    void DisconnectAsync(DisconnectCode code, bool shouldReconnect);
    void Disconnect(DisconnectCode code, bool shouldReconnect);
    bool Send(std::string, std::string, OnSendRespCallback);
    bool Send(std::string, std::string, char *, int, OnSendRespCallback);
    bool SendNoReply(std::string type, std::string payload);
    void SendDisconnect();

    void SetOnConnStatusChange(OnConnStatusChange onStatusChange) { _onStatusChange = onStatusChange; };
    void SetOnServerDisconnect(OnServerDisconnect onServerDisconnect) { _onServerDisconnect = onServerDisconnect; };
    void SetOnSendReq(OnSendReqCallback callback) { _onSendReq = callback; };
    //设置用于HTTPDNS的回调
    void SetHttpDns(GetHttpDns getHttpDns);
    void SetNetType(NetType netType) { _netType = netType; };

    ConnectStatus GetConnectStatus() { return _connectStatus; };

private:
    void __Run();
    int8_t __RunConnect();
    void __RunReadWrite();

    void __WaitDisconnect();

    void __SetConnectStatus(ConnectStatus status);
    OnSendRespCallback __GetCallback(uint16_t messageId);

    void __handleSendReq(SendReq *sendReq);
    void __handleSendResp(SendResp *sendResp);
    void __handleDisconnect(class Disconnect *disconnect);
private:
    std::string _host;
    std::string _backupIp;
    uint16_t _port;
    bool _isTls;
    TcpSocket *_socket;
    TcpReader _reader;
    TcpWriter _writer;
    std::thread *_thread;
    DisconnectCode _disCode;
    ConnectStatus _connectStatus;
    OnConnStatusChange _onStatusChange;
    OnSendReqCallback _onSendReq;
    OnServerDisconnect _onServerDisconnect;
    GetConnectInfo _getConnectInfo;
    NetType _netType;
    std::mutex _mutex, _mapMutex, _timeoutMutex;

    MessageManager _msgManager;
    SocketBreaker _readWriteBreaker, _connectBreaker;
    std::queue<std::unique_ptr<MessageBase>> _messageQueue;
    Alarm _reconnectAlarm; //重连计时器
    Alarm _pingAlarm, _pingTimeoutAlarm; //心跳计时器
    bool _shouldReconnect; //是否自动重连
    bool _shouldNewBreaker, _shouldNewConnBreaker; //是否重建breaker
    bool _timeoutDisconnect;
    int _reconnectCount;
    uint64_t _lastConnTick; //上一次连接时间

    uint16_t _currentMsgId;
    std::map<uint16_t, OnSendRespCallback> _callbackMap;

    std::map<uint16_t, std::unique_ptr<Alarm, ObjectPool<Alarm>::Destructor>> _timeoutAlarmMap;
    ObjectPool<Alarm> _objectPool;

    std::map<DisconnectCode, std::string> _disCodeMap;
};

#endif /* tcp_client_hpp */
