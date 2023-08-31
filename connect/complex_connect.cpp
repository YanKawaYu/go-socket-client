//
//  complex_connect.cpp
//  go-socket-client
//
//  Copyright © 2023 Yan. All rights reserved.
//

#include "complex_connect.hpp"
#include "../socket/socket_select.hpp"
#include "../log/zlogger.hpp"
#include "../common/time_utils.h"
#include <algorithm>

ComplexConnect::ComplexConnect(unsigned int timeout, unsigned int interval,
                               unsigned int errorInterval, unsigned int maxConnect)
:_timeout(timeout), _interval(interval), _errorInterval(errorInterval),
_maxConnect(maxConnect)
{}

static bool __IsConnecting(const TcpConnector* connector) {
    return connector != NULL && connector->Socket() != NULL;
}

TcpSocket* ComplexConnect::ConnectImpatient(const std::vector<AddrInfo> &addrs, SocketBreaker &breaker) {
    if (addrs.empty()) {
        zcwarn2("no address available");
        return NULL;
    }
    //创建连接
    std::vector<TcpConnector *> connectors;
    for (int i=0; i<addrs.size(); i++) {
        TcpConnector *conn;
#ifdef WITH_SSL
        if (addrs[i].isTls) {
            conn = new SSLConnector(&addrs[i], i, _timeout);
        }else {
#endif
            conn = new TcpConnector(&addrs[i], i, _timeout);
#ifdef WITH_SSL
        }
#endif
        connectors.push_back(conn);
    }

    uint64_t curTime = gettickcount();
    uint64_t lastStartConnTime = curTime - _interval;

    int lastError = 0;
    //当前到第几个服务器
    unsigned int index = 0;
    //结果
    unsigned int retIndex = -1;
    TcpSocket *retSocket = NULL;

    while (1) {
        curTime = gettickcount();

        SocketSelect sel(breaker);
        sel.PreSelect();
        //根据上一次连接是否出错来定间隔时间
        int interval = (lastError == 0) ? _interval : _errorInterval;
        //下一次连接的时间
        int nextConnTimeout = interval - (int)(curTime - lastStartConnTime);
        int timeout = (int)_timeout;
        //正在连接中计数
        unsigned int runningCount = (unsigned int)std::count_if(connectors.begin(), connectors.end(), &__IsConnecting);

        //没有超出服务器总数，正在连接数小于最大连接数
        if (index < connectors.size() && runningCount < _maxConnect) {
            //如果下一次连接还没到
            if (nextConnTimeout > 0) {
                //找出下一次连接与新连接超时谁先到
                timeout = std::min(timeout, nextConnTimeout);
            }
            //如果下一次连接到了
            else {
                lastStartConnTime = gettickcount();
                lastError = 0;
                //下一个服务器
                index++;
            }
        }
        //遍历当前正在连接的服务器
        for (unsigned int i=0; i<index; i++) {
            //如果连接已结束，跳过
            if (connectors[i] == NULL) continue;
            connectors[i]->PreSelect(sel);
            //找到距离超时最近的连接
            timeout = std::min(timeout, connectors[i]->Timeout());
        }
        //确保timeout不会小于0
        timeout = std::max(0, timeout);
        //毫秒转为秒
        int selTimeout = timeout/1000;
        int ret = sel.Select(selTimeout);
        //select出错
        if (ret < 0) {
            zcerror2("select error, socket_errno:%d", errno);
            break;
        }
        //breaker出错
        if (sel.IsException()) {
            zcerror2("select breaker error, socket_errno:%d", errno);
            break;
        }
        //如果有断开连接信号
        if (sel.IsBreak()) {
            zcinfo2("stop connecting");
            break;
        }
        //遍历当前正在连接的服务器
        for (unsigned int i=0; i<index; i++) {
            //如果连接已结束，跳过
            if (connectors[i] == NULL) continue;
            connectors[i]->AfterSelect(sel);
            //检查连接状态
            ConnectorStatus status = connectors[i]->Status();
            //如果连接出错
            if (status == ConnStatusError) {
                connectors[i]->Close();
                delete connectors[i];
                connectors[i] = NULL;
                //标记出错
                lastError = -1;
                continue;
            }
            //如果连接成功
            else if (status == ConnStatusConnected) {
                retSocket = connectors[i]->Socket();
                retIndex = i;
                //避免连接被关闭
                connectors[i]->SetSocket(NULL);
                delete connectors[i];
                connectors[i] = NULL;
                break;
            }
        }
        //检查所有连接是否都结束
        bool allInvalid = true;
        for (TcpConnector *conn : connectors) {
            if (conn != NULL) {
                allInvalid = false;
                break;
            }
        }
        //如果所有连接已结束，或者已经有连接上的服务器
        if (allInvalid || retSocket != NULL) break;
    }
    //关闭所有未结束的连接
    for (unsigned int i=0; i<connectors.size(); i++) {
        if (connectors[i] != NULL) {
            connectors[i]->Close();
            delete connectors[i];
            connectors[i] = NULL;
        }
    }
    connectors.clear();
    return retSocket;
}
