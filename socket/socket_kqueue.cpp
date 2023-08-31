//
//  socket_kqueue.cpp
//  go-socket-client
//
//  Copyright © 2023 Yan. All rights reserved.
//

#include "socket_kqueue.hpp"

#ifdef __APPLE__

KqueueEvent::KqueueEvent(struct kevent pollEvent) {
    _pollEvent = pollEvent;
}
bool KqueueEvent::Readable() const { return _pollEvent.filter == EVFILT_READ; }
bool KqueueEvent::Writable() const { return _pollEvent.filter == EVFILT_WRITE; }
bool KqueueEvent::Error() const { return _pollEvent.flags & EV_ERROR; }
bool KqueueEvent::Invalid() const { return _pollEvent.flags & EV_EOF; }
SOCKET KqueueEvent::FD() const { return (int)(intptr_t)_pollEvent.udata; }

SocketKqueue::SocketKqueue(SocketBreaker& breaker):SocketPollBase(breaker) {
    _kq = kqueue();
    ReadEvent(breaker.BreakerFd(), true);
}

SocketKqueue::~SocketKqueue() {
    close(_kq);
}

void SocketKqueue::ReadEvent(int fd, bool active) {
    auto ret = std::find_if(_events.begin(), _events.end(), [&fd](const struct kevent &v){
        return (int)(intptr_t)v.udata == fd && v.filter == EVFILT_READ;
    });
    //如果没找到，新增一个
    if (ret == _events.end()) {
        if (active) {
            struct kevent event;
            EV_SET(&event, fd, EVFILT_READ, EV_ADD|EV_ENABLE, 0, 0, (void*)(intptr_t)fd);
            _events.push_back(event);
        }
    }else {
        if (active) {
            EV_SET(&(*ret), fd, EVFILT_READ, EV_ADD|EV_ENABLE, 0, 0, (void*)(intptr_t)fd);
        }else {
            EV_SET(&(*ret), fd, EVFILT_READ, EV_DELETE, 0, 0, (void*)(intptr_t)fd);
        }
    }
}

void SocketKqueue::WriteEvent(int fd, bool active) {
    auto ret = std::find_if(_events.begin(), _events.end(), [&fd](const struct kevent &v){
        return (int)(intptr_t)v.udata == fd && v.filter == EVFILT_WRITE;
    });
    //如果没找到，新增一个
    if (ret == _events.end()) {
        if (active) {
            struct kevent event;
            EV_SET(&event, fd, EVFILT_WRITE, EV_ADD|EV_ENABLE, 0, 0, (void*)(intptr_t)fd);
            _events.push_back(event);
        }
    }else {
        if (active) {
            EV_SET(&(*ret), fd, EVFILT_WRITE, EV_ADD|EV_ENABLE, 0, 0, (void*)(intptr_t)fd);
        }else {
            EV_SET(&(*ret), fd, EVFILT_WRITE, EV_DELETE, 0, 0, (void*)(intptr_t)fd);
        }
    }
}

void SocketKqueue::ErrorEvent(SOCKET fd, bool active) {}

void SocketKqueue::ClearEvent() {
    _events.erase(_events.begin()+1, _events.end());
}

int SocketKqueue::Poll() {
    return Poll(-1);
}

int SocketKqueue::Poll(int timeout) {
    //将秒转为毫秒
    if (timeout > 0) timeout *= 1000;
    if (timeout < -1) timeout = 0;
    //重置
    _errno = 0;
    _pollEvents.clear();
    //超时时间
    struct timespec timeSpec;
    timeSpec.tv_sec = timeout/1000;
    timeSpec.tv_nsec = (timeout%1000)*1000*1000;
    struct timespec *pTimeSpec = &timeSpec;
    //如果没有超时时间，参数为NULL
    if (timeout == -1) {
        pTimeSpec = NULL;
    }
    //注意，超时时间
    int eventNum = (int)_events.size();
    struct kevent retEvents[_events.size()];
    int ret = kevent(_kq, &_events[0], eventNum, retEvents, eventNum, pTimeSpec);
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

bool SocketKqueue::ReadFdIsSet(SOCKET fd) const {
    for (auto event : _pollEvents) {
        if (event.FD() == fd &&
            !event.Error() && !event.Invalid() &&
            event.Readable()) {
            return true;
        }
    }
    return 0;
}

bool SocketKqueue::WriteFdIsSet(SOCKET fd) const {
    for (auto event : _pollEvents) {
        if (event.FD() == fd &&
            !event.Error() && !event.Invalid() &&
            event.Writable()) {
            return true;
        }
    }
    return 0;
}

bool SocketKqueue::ExceptionFdIsSet(SOCKET fd) const {
    for (auto event : _pollEvents) {
        if (event.FD() == fd) {
            return event.Error() || event.Invalid();
        }
    }
    return 0;
}

bool SocketKqueue::BreakerIsError() const {
    return ExceptionFdIsSet(_breaker.BreakerFd());
}

bool SocketKqueue::BreakerIsBreak() const {
    return ReadFdIsSet(_breaker.BreakerFd());
}

#endif /* __APPLE__ */
