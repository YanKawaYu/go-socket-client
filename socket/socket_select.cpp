//
//  socket_select.cpp
//  go-socket-client
//
//  Copyright © 2023 Yan. All rights reserved.
//

#include "socket_select.hpp"
#ifdef __APPLE__
#include "socket_kqueue.hpp"
#else
#include "socket_epoll.hpp"
#endif

SocketSelect::SocketSelect(SocketBreaker& breaker) {
#ifdef __APPLE__
    _socketPoll = new SocketKqueue(breaker);
#else
    _socketPoll = new SocketEpoll(breaker);
#endif
}

SocketSelect::~SocketSelect() {
    delete _socketPoll;
}

void SocketSelect::PreSelect() {
    _socketPoll->ClearEvent();
}

int SocketSelect::Select() {
    return Select(-1);
}

int SocketSelect::Select(int timeout) {
    return _socketPoll->Poll(timeout);
}

void SocketSelect::ReadFdSet(SOCKET socket) {
    _socketPoll->ReadEvent(socket, true);
}

void SocketSelect::WriteFdSet(SOCKET socket) {
    _socketPoll->WriteEvent(socket, true);
}

void SocketSelect::ExceptionFdSet(SOCKET socket) {
    _socketPoll->ErrorEvent(socket, true);
}

bool SocketSelect::ReadFdIsSet(SOCKET socket) const {
    return _socketPoll->ReadFdIsSet(socket);
}

bool SocketSelect::WriteFdIsSet(SOCKET socket) const {
    return _socketPoll->WriteFdIsSet(socket);
}

bool SocketSelect::ExceptionFdIsSet(SOCKET socket) const {
    return _socketPoll->ExceptionFdIsSet(socket);
}
