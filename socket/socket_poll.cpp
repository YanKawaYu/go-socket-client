//
//  socket_pull.cpp
//  go-socket-client
//
//  Copyright © 2023 Yan. All rights reserved.
//

#include "socket_poll.hpp"
#include <errno.h>
#include <algorithm>

PollEvent::PollEvent(pollfd pollEvent) {
    _pollEvent = pollEvent;
}
bool PollEvent::Readable() const { return _pollEvent.revents & POLLIN; }
bool PollEvent::Writable() const { return _pollEvent.revents & POLLOUT; }
bool PollEvent::HangUp() const { return _pollEvent.revents & POLLHUP; }
bool PollEvent::Error() const { return _pollEvent.revents & POLLERR; }
bool PollEvent::Invalid() const { return _pollEvent.revents & POLLNVAL; }
SOCKET PollEvent::FD() const { return _pollEvent.fd; }

SocketPoll::SocketPoll(SocketBreaker& breaker):SocketPollBase(breaker) {
    //监听breaker
    _events.push_back({
        breaker.BreakerFd(), POLLIN, 0
    });
}

SocketPoll::~SocketPoll() {}

void SocketPoll::ReadEvent(SOCKET fd, bool active) {
    auto ret = std::find_if(_events.begin(), _events.end(), [&fd](const pollfd &v){
        return v.fd == fd;
    });
    //如果没找到，新增一个
    if (ret == _events.end()) {
        __AddEvent(fd, active?true:false, false, false);
    }else {
        if (active) {
            ret->events |= POLLIN;
        }else {
            ret->events &= ~POLLIN;
        }
    }
}

void SocketPoll::WriteEvent(SOCKET fd, bool active) {
    auto ret = std::find_if(_events.begin(), _events.end(), [&fd](const pollfd &v){
        return v.fd == fd;
    });
    //如果没找到，新增一个
    if (ret == _events.end()) {
        __AddEvent(fd, false, active?true:false, false);
    }else {
        if (active) {
            ret->events |= POLLOUT;
        }else {
            ret->events &= ~POLLOUT;
        }
    }
}

void SocketPoll::ErrorEvent(SOCKET fd, bool active) {
    auto ret = std::find_if(_events.begin(), _events.end(), [&fd](const pollfd &v){
        return v.fd == fd;
    });
    //如果没找到，新增一个
    if (ret == _events.end()) {
        __AddEvent(fd, false, false, active?true:false);
    }else {
        if (active) {
            ret->events |= POLLERR;
            ret->events |= POLLNVAL;
        }else {
            ret->events &= ~POLLERR;
            ret->events &= ~POLLNVAL;
        }
    }
}

void SocketPoll::__AddEvent(SOCKET fd, bool read, bool write, bool error) {
    auto ret = std::find_if(_events.begin(), _events.end(), [&fd](const pollfd &v){
        return v.fd == fd;
    });
    short events = 0;
    events |= read?POLLIN:0;
    events |= write?POLLOUT:0;
    events |= error?(POLLERR|POLLNVAL):0;
    pollfd addEvent = {
        fd, events, 0
    };
    if (ret == _events.end()) {
        _events.push_back(addEvent);
    }else {
        *ret = addEvent;
    }
}

void SocketPoll::ClearEvent() {
    _events.erase(_events.begin()+1, _events.end());
}

int SocketPoll::Poll() {
    return Poll(-1);
}

int SocketPoll::Poll(int timeout) {
    //将秒转为毫秒
    if (timeout > 0) timeout *= 1000;
    if (timeout < -1) timeout = 0;
    //重置
    _errno = 0;
    _pollEvents.clear();
    for (auto &event : _events) {
        event.revents = 0;
    }
    //TODO 改为epoll，效率更高
    //注意，超时时间是毫秒
    int ret = poll(&_events[0], (nfds_t)_events.size(), timeout);
    do {
        //如果出错
        if (ret < 0) {
            _errno = errno;
            break;
        }
        //如果超时
        if (ret == 0) {
            break;
        }
        //遍历所有事件，找到触发的
        for (auto &event : _events) {
            if (event.revents == 0) continue;
            _pollEvents.push_back(PollEvent(event));
        }
    }while(0);
    //重置breaker
    _breaker.Clear();
    return ret;
}

bool SocketPoll::ReadFdIsSet(SOCKET fd) const {
    for (auto event : _pollEvents) {
        if (event.FD() == fd) {
            return event.Readable() || event.HangUp();
        }
    }
    return 0;
}

bool SocketPoll::WriteFdIsSet(SOCKET fd) const {
    for (auto event : _pollEvents) {
        if (event.FD() == fd) {
            return event.Writable();
        }
    }
    return 0;
}

bool SocketPoll::ExceptionFdIsSet(SOCKET fd) const {
    for (auto event : _pollEvents) {
        if (event.FD() == fd) {
            return event.Error() || event.Invalid();
        }
    }
    return 0;
}

bool SocketPoll::BreakerIsError() const {
    PollEvent logic_event = _events[0];
    return logic_event.Invalid() || logic_event.Error();
}

bool SocketPoll::BreakerIsBreak() const {
    PollEvent logic_event = _events[0];
    return logic_event.Readable();
}
