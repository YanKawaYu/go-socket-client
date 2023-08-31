//
//  socket_breaker.cpp
//  go-socket-client
//
//  Copyright © 2023 Yan. All rights reserved.
//

#include "socket_breaker.hpp"
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <string>
#include <iostream>

SocketBreaker::SocketBreaker() {
    ReCreate();
}

SocketBreaker::~SocketBreaker() {
    Close();
}

bool SocketBreaker::ReCreate() {
    _pipes[0] = -1;
    _pipes[1] = -1;
    //创建管道
    int ret = pipe(_pipes);
    if (ret != 0) {
        _pipes[0] = -1;
        _pipes[1] = -1;
        std::cout << "create pipe failed" << std::endl;
        return false;
    }
    //读取管道的当前状态
    long flags0 = fcntl(_pipes[0], F_GETFL);
    long flags1 = fcntl(_pipes[1], F_GETFL);
    if (flags0 < 0 || flags1 < 0) {
        Close();
        std::cout << "get pipe state failed" << std::endl;
        return false;
    }
    //设置管道为非阻塞模式
    int ret0 = fcntl(_pipes[0], F_SETFL, flags0|O_NONBLOCK);
    int ret1 = fcntl(_pipes[1], F_SETFL, flags1|O_NONBLOCK);
    if (ret0 == -1 || ret1 == -1) {
        Close();
        std::cout << "set pipe state failed" << std::endl;
        return false;
    }
    return true;
}

void SocketBreaker::Close() {
    if (_pipes[0] != -1) {
        close(_pipes[0]);
    }
    if (_pipes[1] != -1) {
        close(_pipes[1]);
    }
}

bool SocketBreaker::Break() {
    std::lock_guard<std::mutex> lock(_mutex);
    //如果已经发送信号了，直接返回
    if (_broken) return true;
    //通过向管道写入1来发送信号
    const char dummy = '1';
    int ret = (int)write(_pipes[1], &dummy, sizeof(dummy));
    //写入失败
    if (ret != (int)sizeof(dummy)) {
        return false;
    }
    _broken = true;
    return _broken;
}

bool SocketBreaker::Clear() {
    std::lock_guard<std::mutex> lock(_mutex);
    //并发的时候管道可能有多个信号，故设置为128确保全部清空
    char dummy[128];
    //通过从管道中读取来清除信号
    int ret = (int)read(_pipes[0], dummy, sizeof(dummy));
    if (ret < 0) {
        return false;
    }
    _broken = false;
    return true;
}
