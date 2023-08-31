//
//  tcp_connector.cpp
//  go-socket-client
//
//  Copyright © 2023 Yan. All rights reserved.
//

#include "tcp_connector.hpp"
#include "log/zlogger.hpp"
#include "../common/time_utils.h"
#include <cstring>
#include <climits>

TcpConnector::TcpConnector(const AddrInfo *addr, unsigned int index, unsigned int connTimeout)
: _addr(addr), _socket(NULL), _connTimeout(connTimeout),
_startConnTime(0), _status(ConnStatusStart), _index(index)
{}

TcpConnector::~TcpConnector() {
    Close();
}

int TcpConnector::Timeout() const {
    //正在TCP连接或者正在SSL连接
    if (_status == ConnStatusConnecting || _status == ConnStatusSSLConnecting) {
        //连接剩余超时时间
        return (int)(_startConnTime + _connTimeout - gettickcount());
    }else if (_status == ConnStatusError || _status == ConnStatusConnected) {
        return 0;
    }else {
        return INT_MAX;
    }
}

void TcpConnector::Close() {
    if (_socket == NULL) return;
    _socket->Close();
    delete _socket;
    _socket = NULL;
    _status = ConnStatusEnd;
}

TcpSocket * TcpConnector::__NewSocket(SOCKET socket) {
    return new TcpSocket(socket);
}

void TcpConnector::PreConnectSelect(SocketSelect &sel) {
    zcinfo2("connector %d: start connect %s:%d", _index, _addr->ip.c_str(), _addr->port);
    SOCKET socketFd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (socketFd == INVALID_SOCKET) {
        zcerror2("connector %d: invalid socket", _index);
        _status = ConnStatusError;
        return;
    }
    if (_addr->ip == "") {
        zcwarn2("connector %d: empty host", _index);
        _status = ConnStatusError;
        return;
    }
    //设置socket为非阻塞模式，用于检查连接是否超时
    if (socket_set_noblo(socketFd) != 0) {
        zcerror2("connector %d: set socket nonblock failed:%d", _index, errno);
        _status = ConnStatusError;
        return;
    }
    //记录时间
    _startConnTime = gettickcount();
    //连接服务器
    struct sockaddr_in serv_addr;
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = inet_addr(_addr->ip.c_str());
    serv_addr.sin_port = htons(_addr->port);
    int ret = connect(socketFd, (sockaddr*)&serv_addr, sizeof(serv_addr));
    if (ret != 0 && !IS_NOBLOCK_CONNECT_ERRNO(errno)) {
        zcerror2("connector %d: connect error, socket_errno:%d", _index, errno);
        _status = ConnStatusError;
        return;
    }
    sel.WriteFdSet(socketFd);
    sel.ExceptionFdSet(socketFd);
    _socket = __NewSocket(socketFd);
    //正在连接中
    _status = ConnStatusConnecting;
}

void TcpConnector::AfterConnectSelect(SocketSelect &sel) {
    if (_socket == NULL) {
        zcerror2("unexpected empty socket");
        _status = ConnStatusError;
        return;
    }
    int timeout = Timeout();
    SOCKET socketFd = _socket->GetSocket();
    if (sel.ExceptionFdIsSet(socketFd)) {
        int status = 0;
        socklen_t len = sizeof(status);
        int ret = getsockopt(socketFd, SOL_SOCKET, SO_ERROR, &status, &len);
        if (ret != 0) {
            zcerror2("connector %d: get socketopt error", _index);
        }else {
            zcerror2("connector %d: fd error:%d, str:%s", _index, status, strerror(status));
        }
        _status = ConnStatusError;
        return;
    }
    //连接成功
    if (sel.WriteFdIsSet(socketFd)) {
        //记录连接时间
        _endConnTime = gettickcount();
        int totalCost = (int)(_endConnTime - _startConnTime);
        zcinfo2("connector %d finish tcp:%d ms", _index, totalCost);
        //如果需要TLS，继续TLS认证
        if (_addr->isTls) {
            _status = ConnStatusStartSSL;
        }
        //如果不需要TLS，直接标记为已连接
        else {
            //恢复socket为阻塞
            if (socket_set_blo(socketFd) != 0) {
                zcerror2("connector %d: set socket block failed:%d", _index, errno);
                _status = ConnStatusError;
                return;
            }
            _status = ConnStatusConnected;
        }
        return;
    }
    //连接超时
    if (timeout <= 0) {
        zcerror2("connector %d: timeout", _index);
        _status = ConnStatusError;
        return;
    }
}

void TcpConnector::PreSelect(SocketSelect &sel) {
    switch (_status) {
        case ConnStatusStart:
            PreConnectSelect(sel);
            break;
        case ConnStatusConnecting:{
            SOCKET socket = _socket->GetSocket();
            sel.WriteFdSet(socket);
            sel.ExceptionFdSet(socket);
            break;
        }
        default:
            break;
    }
}

void TcpConnector::AfterSelect(SocketSelect &sel) {
    switch (_status) {
        case ConnStatusConnecting:
            AfterConnectSelect(sel);
            break;
        default:
            break;
    }
}
