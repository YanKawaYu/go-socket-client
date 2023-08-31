//
//  socket_pull.hpp
//  go-socket-client
//
//  Copyright © 2023 Yan. All rights reserved.
//

#ifndef socket_pull_hpp
#define socket_pull_hpp

#include <poll.h>
#include <stdio.h>
#include <vector>
#include "socket_poll_base.hpp"

class PollEvent {
public:
    PollEvent(pollfd);

    bool Readable() const;
    bool Writable() const;
    bool HangUp() const;
    bool Error() const;
    bool Invalid() const;

    SOCKET FD() const;
private:
    pollfd _pollEvent;
};

class SocketPoll : public SocketPollBase {
public:
    SocketPoll(SocketBreaker& breaker);
    ~SocketPoll();

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
    void __AddEvent(SOCKET fd, bool read, bool write, bool error);
private:
    std::vector<pollfd> _events;
    std::vector<PollEvent> _pollEvents;
};

#endif /* socket_pull_hpp */
