//
//  tcp_io.hpp
//  go-socket-client
//
//  Copyright © 2023 Yan. All rights reserved.
//

#ifndef tcp_io_hpp
#define tcp_io_hpp

#include <stdio.h>
#include "tcp_error.hpp"
#include "unix_socket.hpp"
#include "../socket/tcp_socket.hpp"

class IReader {
public:
    virtual void ReadFull(void *, size_t) = 0;
};

class IWriter {
public:
    virtual void Write(void *, size_t) = 0;
};

class TcpReader : public IReader {
public:
    TcpReader();
    ~TcpReader();
public:
    void SetSocket(TcpSocket *socket) { _socket = socket; };
    ssize_t __Read(void *, size_t);
    void ReadFull(void *, size_t) override;
private:
    TcpSocket *_socket;
};

class TcpWriter : public IWriter {
public:
    TcpWriter();
    ~TcpWriter();
public:
    void SetSocket(TcpSocket *socket) { _socket = socket; };
    void Write(void *, size_t) override;
private:
    TcpSocket *_socket;
};

#endif /* tcp_io_hpp */
