//
//  socket_breaker.hpp
//  go-socket-client
//
//  Copyright © 2023 Yan. All rights reserved.
//

#ifndef socket_breaker_hpp
#define socket_breaker_hpp

#include <stdio.h>
#include <mutex>

class SocketBreaker {
public:
    SocketBreaker();
    ~SocketBreaker();
public:
    bool ReCreate(); //创建管道
    void Close(); //关闭管道

    bool Break(); //发送信号
    bool Clear(); //清除信号

    int BreakerFd() { return _pipes[0]; }; //获取管道的读取文件描述符
    bool IsBreak() { return _broken; } //是否有信号
private:
    int _pipes[2];
    bool _broken = false;
    std::mutex _mutex; //必须加锁，否则并发会出问题
};

#endif /* socket_breaker_hpp */
