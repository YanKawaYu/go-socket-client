//
//  alarm.cpp
//  go-socket-client
//
//  Copyright © 2023 Yan. All rights reserved.
//

#include "alarm.hpp"
#include "message_queue.hpp"

Alarm::Alarm(): _callback(NULL),
    _innerCallback(NULL),
    _messageId(0),
    _status(kStatusInit) {}

Alarm::~Alarm() {
    Cancel();
}

void Alarm::SetCallback(OnAlarmCallback callback) {
    //必须持有回调，否则可能会引发野指针
    _innerCallback = callback;
    _callback = [this]() {
        //对象已被释放的话，回调再被执行的时候会闪退，这里暂不考虑
        //这里必须要有锁，否则可能_messageId还没被赋值，回调就已被执行
        std::unique_lock<std::mutex> lock(this->_mutex);
        //回调与取消回调可能并发，如果当前对象已被取消
        if (_messageId == 0) return;
        this->_status = kStatusAlarm;
        //回调的时候不能持有锁，否则容易死锁
        lock.unlock();
        this->_innerCallback();
    };
}

bool Alarm::Start(int afterSec) {
    std::lock_guard<std::mutex> lock(_mutex);
    //未设置回调
    if (_callback == NULL) return false;
    //已经开始过了
    if (_status == kStatusStart) return false;
    _messageId = MessageQueue::SharedQueue()->PostMessage(afterSec, _callback);
    _status = kStatusStart;
    return true;
}

bool Alarm::Start(int afterSec, OnAlarmCallback callback) {
    SetCallback(callback);
    return Start(afterSec);
}

bool Alarm::Cancel() {
    std::lock_guard<std::mutex> lock(_mutex);
    if (_messageId == 0) return false;
    MessageQueue::SharedQueue()->CancelMessage(_messageId);
    _messageId = 0;
    _status = kStatusCancel;
    return true;
}

bool Alarm::IsWaiting() const {
    return _status == kStatusStart;
}

bool Alarm::IsAlarm() const {
    return _status == kStatusAlarm;
}
