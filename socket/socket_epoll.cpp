//
//  socket_epoll.cpp
//  go-socket-client
//
//  Copyright © 2023 Yan. All rights reserved.
//

#include "socket_epoll.hpp"
#include <algorithm>

#ifndef __APPLE__

EpollEvent::EpollEvent(struct epoll_event pollEvent) {
    _pollEvent = pollEvent;
}

bool EpollEvent::Readable() const { return _pollEvent.events & EPOLLIN; }
bool EpollEvent::Writable() const { return _pollEvent.events & EPOLLOUT; }
bool EpollEvent::HangUp() const { return _pollEvent.events & EPOLLHUP; }
bool EpollEvent::Error() const { return _pollEvent.events & EPOLLERR; }
SOCKET EpollEvent::FD() const { return _pollEvent.data.fd; }

SocketEpoll::SocketEpoll(SocketBreaker& breaker):SocketPollBase(breaker) {
    //Since Linux 2.6.8,the size argument is ignored, but must be greater than zero
    _epfd = epoll_create(1);
    ReadEvent(breaker.BreakerFd(), true);
}

SocketEpoll::~SocketEpoll() {
    close(_epfd);
}

void SocketEpoll::ReadEvent(int fd, bool active) {
    auto ret = std::find_if(_events.begin(), _events.end(), [&fd](const struct epoll_event &v){
        return v.data.fd == fd;
    });
    //如果没找到，新增一个
    if (ret == _events.end()) {
        struct epoll_event event;
        event.data.fd = fd;
        event.events = active?EPOLLIN:0;
        _events.push_back(event);
    }else {
        if (active) {
            ret->events |= EPOLLIN;
        }else {
            ret->events &= ~EPOLLIN;
        }
    }
}

void SocketEpoll::WriteEvent(int fd, bool active) {
    auto ret = std::find_if(_events.begin(), _events.end(), [&fd](const struct epoll_event &v){
        return v.data.fd == fd;
    });
    //如果没找到，新增一个
    if (ret == _events.end()) {
        struct epoll_event event;
        event.data.fd = fd;
        event.events = active?EPOLLOUT:0;
        _events.push_back(event);
    }else {
        if (active) {
            ret->events |= EPOLLOUT;
        }else {
            ret->events &= ~EPOLLOUT;
        }
    }
}

void SocketEpoll::ErrorEvent(SOCKET fd, bool active) {
    auto ret = std::find_if(_events.begin(), _events.end(), [&fd](const epoll_event &v){
        return v.data.fd == fd;
    });
    //如果没找到，新增一个
    if (ret == _events.end()) {
        struct epoll_event event;
        event.data.fd = fd;
        event.events = active?EPOLLERR:0;
        _events.push_back(event);
    }else {
        if (active) {
            ret->events |= EPOLLERR;
        }else {
            ret->events &= ~EPOLLERR;
        }
    }
}

void SocketEpoll::ClearEvent() {
    _events.erase(_events.begin()+1, _events.end());
}

int SocketEpoll::Poll() {
    return Poll(-1);
}

int SocketEpoll::Poll(int timeout) {
    //将秒转为毫秒
    if (timeout > 0) timeout *= 1000;
    if (timeout < -1) timeout = 0;
    //重置
    _errno = 0;
    _pollEvents.clear();
    int eventNum = (int)_events.size();
    //监听事件
    for (int i=0; i<eventNum; i++) {
        epoll_ctl(_epfd, EPOLL_CTL_ADD, _events[i].data.fd, &_events[i]);
    }
    struct epoll_event retEvents[eventNum];
    //注意，超时时间
    int ret = epoll_wait(_epfd, retEvents, eventNum, timeout);
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
        for (int i=0; i<ret; i++) {
            _pollEvents.push_back(retEvents[i]);
        }
    }while(0);
    //重置breaker
    _breaker.Clear();
    return ret;
}

bool SocketEpoll::ReadFdIsSet(SOCKET fd) const {
    for (auto event : _pollEvents) {
        if (event.FD() == fd) {
            return event.Readable() || event.HangUp();
        }
    }
    return 0;
}

bool SocketEpoll::WriteFdIsSet(SOCKET fd) const {
    for (auto event : _pollEvents) {
        if (event.FD() == fd) {
            return event.Writable();
        }
    }
    return 0;
}

bool SocketEpoll::ExceptionFdIsSet(SOCKET fd) const {
    for (auto event : _pollEvents) {
        if (event.FD() == fd) {
            return event.Error();
        }
    }
    return 0;
}

bool SocketEpoll::BreakerIsError() const {
    return ExceptionFdIsSet(_breaker.BreakerFd());
}

bool SocketEpoll::BreakerIsBreak() const {
    return ReadFdIsSet(_breaker.BreakerFd());
}

#endif /* not apple */
