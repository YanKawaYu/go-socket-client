//
//  socket_select.hpp
//  go-socket-client
//
//  Copyright © 2023 Yan. All rights reserved.
//

#ifndef socket_select_hpp
#define socket_select_hpp

#include <stdio.h>
#include "socket_breaker.hpp"
#include "socket_poll_base.hpp"

class SocketSelect {
public:
    SocketSelect(SocketBreaker& breaker);
    ~SocketSelect();

    void PreSelect();
    int Select();
    int Select(int timeout);

    void ReadFdSet(SOCKET socket);
    void WriteFdSet(SOCKET socket);
    void ExceptionFdSet(SOCKET socket);

    bool ReadFdIsSet(SOCKET socket) const;
    bool WriteFdIsSet(SOCKET socket) const;
    bool ExceptionFdIsSet(SOCKET socket) const;

    bool IsBreak() const { return _socketPoll->BreakerIsBreak(); };
    bool IsException() const { return _socketPoll->BreakerIsError(); };
    int GetErrno() const { return _socketPoll->GetErrno(); };
private:
    SocketPollBase *_socketPoll;
};

#endif /* socket_select_hpp */
