//
//  socket_epoll.hpp
//  go-socket-client
//
//  Copyright © 2023 Yan. All rights reserved.
//

#ifndef socket_epoll_hpp
#define socket_epoll_hpp

#ifndef __APPLE__

#include <stdio.h>
#include <vector>
#include <sys/epoll.h>
#include "socket_poll_base.hpp"

class EpollEvent {
public:
    EpollEvent(struct epoll_event);

    bool Readable() const;
    bool Writable() const;
    bool HangUp() const;
    bool Error() const;

    SOCKET FD() const;
private:
    struct epoll_event _pollEvent;
};

class SocketEpoll : public SocketPollBase {
public:
    SocketEpoll(SocketBreaker& breaker);
    ~SocketEpoll();

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
    int _epfd;
    std::vector<struct epoll_event> _events;
    std::vector<EpollEvent> _pollEvents;
};

#endif /* not apple */

#endif /* socket_epoll_hpp */
