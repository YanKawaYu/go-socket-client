//
//  ssl_socket.cpp
//  go-socket-client
//
//  Copyright © 2023 Yan. All rights reserved.
//

#include "ssl_socket.hpp"
#include "log/zlogger.hpp"
#include "../packet/tcp_error.hpp"

static SSL_CTX *_ctx = NULL;

SSL_CTX * SSLSocket::GetSSLContext() {
    if (_ctx == NULL) {
        SSL_library_init();
        _ctx = SSL_CTX_new(TLS_client_method());
    }
    return _ctx;
}

SSLSocket::SSLSocket(SOCKET socket):TcpSocket(socket), _connectError(0), _ssl(NULL)
{}

int SSLSocket::Connect() {
    _ssl = SSL_new(GetSSLContext());
    if (_ssl == NULL) {
        zcerror2("ssl new error");
        return -1;
    }
    SSL_set_fd(_ssl, _socket);
    //socket是非阻塞的，故正常情况返回-1
    int status = SSL_connect(_ssl);
    //如果已连接成功（正常不会出现）
    if (status == 1) {
        return SSL_ERROR_NONE;
    }
    //获取要监听读还是写
    _connectError = SSL_get_error(_ssl, status);
    return _connectError;
}

int SSLSocket::GetConnectError() {
    return _connectError;
}

void SSLSocket::Close() {
    if (_socket == INVALID_SOCKET) return;
    if (_ssl != NULL) {
        SSL_free(_ssl);
    }
    close(_socket);
    _socket = INVALID_SOCKET;
}

int SSLSocket::Read(void *buf, size_t len) {
    int recvLen = SSL_read(_ssl, buf, (int)len);
    if (recvLen == 0) {
        throw TcpException(TcpException::TcpExcSocketClose);
    }
    if (recvLen < 0) {
        throw TcpException(TcpException::TcpExcSocketError, SSL_get_error(_ssl, recvLen));
    }
    return recvLen;
}

void SSLSocket::Write(void *buf, size_t len) {
    int ret = SSL_write(_ssl, buf, (int)len);
    if (ret <= 0) {
        throw TcpException(TcpException::TcpExcSocketError, SSL_get_error(_ssl, ret));
    }
}
