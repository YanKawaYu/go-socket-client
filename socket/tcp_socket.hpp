//
//  tcp_socket.hpp
//  go-socket-client
//
//  Copyright © 2023 Yan. All rights reserved.
//

#ifndef tcp_socket_hpp
#define tcp_socket_hpp

#include <stdio.h>
#include "../packet/unix_socket.hpp"

class TcpSocket {
public:
    TcpSocket(SOCKET socket);

    bool IsInvalid();
    SOCKET GetSocket();
    //以下方法用于子类重写
    virtual int Connect(); //用于tcp连接建立后进一步的认证，如TLS
    virtual int GetConnectError(); //TLS需要保存上一次连接的错误
    virtual void Close();

    virtual void Write(void *buf, size_t len);
    virtual int Read(void *buf, size_t len);
protected:
    SOCKET _socket;
};

#endif /* tcp_socket_hpp */
