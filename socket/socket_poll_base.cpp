//
//  socket_poll_base.cpp
//  go-socket-client
//
//  Copyright © 2023 Yan. All rights reserved.
//

#include "socket_poll_base.hpp"

SocketPollBase::SocketPollBase(SocketBreaker& breaker) : _breaker(breaker) {}

SocketPollBase::~SocketPollBase() {}
