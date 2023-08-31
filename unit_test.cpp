//
//  unit_test.cpp
//  go-socket-client
//
//  Copyright © 2023 Yan. All rights reserved.
//

#include "unit_test.hpp"
#include "common/alarm.hpp"
#include <unistd.h>
#include <iostream>
#include <chrono>
#include "socket/socket_select.hpp"

#define STD_COUT (std::cout << currentDateTime() << " ")

const std::string currentDateTime() {
    time_t     now = time(0);
    struct tm  tstruct;
    char       buf[80];
    tstruct = *localtime(&now);
    strftime(buf, sizeof(buf), "%X", &tstruct);

    return buf;
}

void testConcurrentAlarm() {
    while (1) {
        STD_COUT << "start alarm" << std::endl;
        Alarm testAlarm;
        testAlarm.Start(1, [](){
            STD_COUT << "alarm fired" << std::endl;
        });
        sleep(1);
    }
}

void testSocketPoll() {
    int pipes[2];
    pipes[0] = -1;
    pipes[1] = -1;
    //创建管道
    pipe(pipes);
    int readFd = pipes[0];
    int writeFd = pipes[1];
//    const char dummy = '1';
//    int ret = write(writeFd, &dummy, sizeof(dummy));

    SocketBreaker breaker;
    SocketSelect sel(breaker);
    sel.PreSelect();
//    sel.ReadFdSet(readFd);
    sel.WriteFdSet(writeFd);
    Alarm testAlarm;
    testAlarm.Start(2, [&breaker](){
        breaker.Break();
        STD_COUT << "alarm fired" << std::endl;
    });
    STD_COUT << "start select" << std::endl;
    int selRet = sel.Select(5);
    //监听出错
    if (selRet < 0) {
        STD_COUT << "select error";
    }
    if (selRet == 0) {
        STD_COUT << "select timeout";
    }
    //breaker出错
    if (sel.IsException()) {
        STD_COUT << "breaker error";
    }
    //如果断开连接，直接返回
    if (sel.IsBreak()) {
        STD_COUT << "breaker break";
    }
    if (sel.ExceptionFdIsSet(readFd)) {
        STD_COUT << "read fd error";
    }
//    if (sel.ReadFdIsSet(readFd)) {
//        STD_COUT << "read fd ready";
//    }
    if (sel.WriteFdIsSet(writeFd)) {
        STD_COUT << "write fd ready";
    }
}
