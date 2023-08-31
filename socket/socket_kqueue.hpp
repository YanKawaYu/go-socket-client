//
//  socket_kqueue.hpp
//  go-socket-client
//
//  Copyright © 2023 Yan. All rights reserved.
//

#ifndef socket_kqueue_hpp
#define socket_kqueue_hpp

#ifdef __APPLE__

#include <stdio.h>
#include <sys/event.h>
#include <vector>
#include "socket_poll_base.hpp"

class KqueueEvent {
public:
    KqueueEvent(struct kevent);

    bool Readable() const;
    bool Writable() const;
    bool Error() const;
    bool Invalid() const;
    SOCKET FD() const;
private:
    struct kevent _pollEvent;
};

class SocketKqueue : public SocketPollBase {
public:
    SocketKqueue(SocketBreaker& breaker);
    ~SocketKqueue();

    void ReadEvent(SOCKET fd, bool active);
    void WriteEvent(SOCKET fd, bool active);
    void ErrorEvent(SOCKET fd, bool active);
    void ClearEvent();
    int Poll();
    int Poll(int timeout);

    bool ReadFdIsSet(SOCKET fd) const;
    bool WriteFdIsSet(SOCKET fd) const;
    bool ExceptionFdIsSet(SOCKET fd) const;

    bool BreakerIsError() const;
    bool BreakerIsBreak() const;
private:
    int _kq;
    std::vector<struct kevent> _events;
    std::vector<KqueueEvent> _pollEvents;
};

#endif /* __APPLE__ */

#endif /* socket_kqueue_hpp */
