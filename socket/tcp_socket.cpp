//
//  tcp_socket.cpp
//  go-socket-client
//
//  Copyright © 2023 Yan. All rights reserved.
//

#include "tcp_socket.hpp"
#include "../packet/tcp_error.hpp"
#include <errno.h>

TcpSocket::TcpSocket(SOCKET socket) {
    _socket = socket;
}

bool TcpSocket::IsInvalid() {
    return _socket == INVALID_SOCKET;
}

SOCKET TcpSocket::GetSocket() {
    return _socket;
}

int TcpSocket::Connect() {
    //tcp不需要连接，故直接成功
    return 1;
}

int TcpSocket::GetConnectError() {
    //tcp不需要再连接，故无错
    return 0;
}

void TcpSocket::Close() {
    if (_socket == INVALID_SOCKET) return;
    close(_socket);
    _socket = INVALID_SOCKET;
}

int TcpSocket::Read(void *buf, size_t len) {
    int recvLen = (int)recv(_socket, buf, len, 0);
    if (recvLen == 0) {
        throw TcpException(TcpException::TcpExcSocketClose);
    }
    if (recvLen < 0) {
        throw TcpException(TcpException::TcpExcSocketError, errno);
    }
    return recvLen;
}

void TcpSocket::Write(void *buf, size_t len) {
    ssize_t sendLen = send(_socket, buf, len, 0);
    if (sendLen < 0) {
        throw TcpException(TcpException::TcpExcSocketError, errno);
    }
}
