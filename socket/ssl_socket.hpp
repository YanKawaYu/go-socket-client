//
//  ssl_socket.hpp
//  go-socket-client
//
//  Copyright © 2023 Yan. All rights reserved.
//

#ifndef ssl_socket_hpp
#define ssl_socket_hpp

#include <stdio.h>
#include <openssl/ssl.h>
#include "tcp_socket.hpp"

class SSLSocket : public TcpSocket {
public:
    SSLSocket(SOCKET socket);

    int Connect();
    int GetConnectError();
    void Close();

    void Write(void *buf, size_t len);
    int Read(void *buf, size_t len);
private:
    SSL *_ssl;
    int _connectError;

    SSL_CTX *GetSSLContext();
};

#endif /* ssl_socket_hpp */
