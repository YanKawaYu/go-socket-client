//
//  socket_poll_base.hpp
//  go-socket-client
//
//  Copyright © 2023 Yan. All rights reserved.
//

#ifndef socket_poll_base_hpp
#define socket_poll_base_hpp

#include <stdio.h>
#include "socket_breaker.hpp"
#include "../packet/unix_socket.hpp"

class SocketPollBase {
public:
    SocketPollBase(SocketBreaker& breaker);
    //必须定义，否则父类指针delete无法调用子类的析构函数
    virtual ~SocketPollBase();

    virtual void ReadEvent(SOCKET fd, bool active) = 0;
    virtual void WriteEvent(SOCKET fd, bool active) = 0;
    virtual void ErrorEvent(SOCKET fd, bool active) = 0;
    virtual void ClearEvent() = 0;
    virtual int Poll() = 0;
    virtual int Poll(int timeout) = 0;

    virtual bool ReadFdIsSet(SOCKET fd) const = 0;
    virtual bool WriteFdIsSet(SOCKET fd) const = 0;
    virtual bool ExceptionFdIsSet(SOCKET fd) const = 0;

    virtual bool BreakerIsError() const = 0;
    virtual bool BreakerIsBreak() const = 0;

    const int GetErrno() const { return _errno; };
protected:
    SocketBreaker& _breaker;
    int _errno;
};

#endif /* socket_poll_base_hpp */
