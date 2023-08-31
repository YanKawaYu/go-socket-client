//
//  alarm.hpp
//  go-socket-client
//
//  Copyright © 2023 Yan. All rights reserved.
//

#ifndef alarm_hpp
#define alarm_hpp

#include <stdio.h>
#include <functional>
#include <mutex>
#include <memory>

typedef std::function<void()> OnAlarmCallback;

/*
 *  这个Alarm用在非单例的成员变量或者局部变量时，当Alarm fired事件与Alarm销毁并发
 *  有概率会闪退，暂时没有好的方法解决，使用的时候一定要注意！！！
 */
class Alarm {
private:
    enum AlarmStatus {
        kStatusInit = 0,
        kStatusStart = 1,
        kStatusCancel,
        kStatusAlarm,
    };
public:
    Alarm();
    ~Alarm();

    bool Start(int afterSec);
    bool Start(int afterSec, OnAlarmCallback callback);
    bool Cancel();
    bool IsWaiting() const;
    bool IsAlarm() const;

    void SetUserData(int userData) { _userData = userData; };
    int GetUserData() { return _userData; };

    void SetCallback(OnAlarmCallback callback);
private:
    OnAlarmCallback _callback;
    OnAlarmCallback _innerCallback;
    uint32_t _messageId;
    AlarmStatus _status;
    int _userData;

    std::mutex _mutex;
};

#endif /* alarm_hpp */
