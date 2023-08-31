//
//  ssl_connector.cpp
//  go-socket-client
//
//  Copyright © 2023 Yan. All rights reserved.
//

#include "ssl_connector.hpp"
#include "log/zlogger.hpp"
#include "../common/time_utils.h"
#include "../socket/ssl_socket.hpp"
#include <cstring>

SSLConnector::SSLConnector(const AddrInfo *addr, unsigned int index, unsigned int connTimeout)
: TcpConnector(addr, index, connTimeout)
{}

TcpSocket * SSLConnector::__NewSocket(SOCKET socket) {
    return new SSLSocket(socket);
}

void SSLConnector::PreSelect(SocketSelect &sel) {
    TcpConnector::PreSelect(sel);
    switch (_status) {
        case ConnStatusStartSSL:
            PreSSLSelect(sel);
            break;
        case ConnStatusSSLConnecting:{
            SOCKET socket = _socket->GetSocket();
            //根据上次SSL的连接结果进行监听
            int connError = _socket->GetConnectError();
            sel.ExceptionFdSet(socket);
            if (connError == SSL_ERROR_WANT_READ) {
                sel.ReadFdSet(socket);
            }else if (connError == SSL_ERROR_WANT_WRITE) {
                sel.WriteFdSet(socket);
            }
            break;
        }
        default:
            break;
    }
}

void SSLConnector::AfterSelect(SocketSelect &sel) {
    TcpConnector::AfterSelect(sel);
    switch (_status) {
        case ConnStatusSSLConnecting:
            AfterSSLSelect(sel);
            break;
        default:
            break;
    }
}

void SSLConnector::PreSSLSelect(SocketSelect &sel) {
    int error = _socket->Connect();
    if (error == SSL_ERROR_WANT_READ) {
        sel.ReadFdSet(_socket->GetSocket());
    }else if (error == SSL_ERROR_WANT_WRITE) {
        sel.WriteFdSet(_socket->GetSocket());
    }else if (error == SSL_ERROR_NONE) {
        //socket为非阻塞，故正常不会走到这里
        zcwarn2("ssl connect success early");
        return;
    }else {
        if (error != -1) {
            zcerror2("pre ssl connect error:%d", error);
        }
        _status = ConnStatusError;
        return;
    }
    sel.ExceptionFdSet(_socket->GetSocket());
    //正在连接中
    _status = ConnStatusSSLConnecting;
}

void SSLConnector::AfterSSLSelect(SocketSelect &sel) {
    int timeout = Timeout();
    SOCKET socket = _socket->GetSocket();
    if (sel.ExceptionFdIsSet(socket)) {
        int status = 0;
        socklen_t len = sizeof(status);
        int ret = getsockopt(socket, SOL_SOCKET, SO_ERROR, &status, &len);
        if (ret != 0) {
            zcerror2("ssl connector %d: get socketopt error", _index);
        }else {
            zcerror2("ssl connector %d: fd error:%d, str:%s", _index, status, strerror(status));
        }
        _status = ConnStatusError;
        return;
    }
    //连接成功
    if (sel.WriteFdIsSet(socket) || sel.ReadFdIsSet(socket)) {
        //记录连接时间
        int totalCost = (int)(gettickcount() - _endConnTime);
        zcinfo2("connector %d finish ssl:%d ms", _index, totalCost);
        //恢复socket为阻塞
        if (socket_set_blo(socket) != 0) {
            zcerror2("ssl connector %d: set socket block failed:%d", _index, errno);
            _status = ConnStatusError;
            return;
        }
        _status = ConnStatusConnected;
        return;
    }
    //连接超时
    if (timeout <= 0) {
        zcerror2("connector %d: timeout", _index);
        _status = ConnStatusError;
        return;
    }
}
