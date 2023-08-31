//
//  message_queue.cpp
//  go-socket-client
//
//  Copyright © 2023 Yan. All rights reserved.
//

#include "message_queue.hpp"
#include <thread>
#include <stdint.h>
#include <map>
#include <iostream>

//时间间隔转毫秒
#define DURATION_TO_MS std::chrono::duration_cast<std::chrono::milliseconds>

MessageQueue* MessageQueue::_instance = NULL;

MessageQueue* MessageQueue::SharedQueue() {
    static std::once_flag onceFlag;
    std::call_once(onceFlag, []{
        MessageQueue::_instance = new MessageQueue();
    });
    return MessageQueue::_instance;
}

MessageQueue::MessageQueue()
    : _currentMsgId(1)
    , _firstMessage(NULL)
    , _hasPost(false) {
    std::thread queueThread(&MessageQueue::__RunLoop, this);
    queueThread.detach();
}

void MessageQueue::__RunLoop() {
    while (1) {
        std::unique_lock<std::mutex> lock(_mutex);
        Message *currentMessage = _firstMessage;
        std::chrono::system_clock::time_point now = std::chrono::system_clock::now();
        std::map<uint32_t, MessageCallback> callbackMap;
        //默认用纳秒比较，故这里必须转为毫秒比较，与后面保持一致
        while (currentMessage != NULL && DURATION_TO_MS(now-currentMessage->_when).count() >= 0) {
            Message *deleteMessage = currentMessage;
            //执行回调
            if (currentMessage->_callback != NULL) {
                callbackMap[currentMessage->_messageId] = currentMessage->_callback;
            }
            //检查下一条消息
            currentMessage = currentMessage->_next;
            //将链表头移到下一个
            _firstMessage = currentMessage;
            //删除旧消息
            delete deleteMessage;
        }
        //如果没有消息了
        std::chrono::milliseconds duration;
        if (currentMessage == NULL) {
            duration = std::chrono::milliseconds(0);
        }
        //如果有消息，block到该消息的时间点
        else {
            duration = DURATION_TO_MS(currentMessage->_when-now);
        }
        //清空取消的消息id，必须在解锁之前，确保数组中的消息都是在解锁期间取消的
        _canceledMessageIds.clear();
        //解锁之前重置notify标识
        _hasPost = false;
        //回调之前必须解锁，否则容易产生死锁
        lock.unlock();
        for (auto &kv : callbackMap) {
            lock.lock();
            //有可能在执行回调的过程中message被取消了，这里把取消的消息跳过，避免非法访问
            bool isExist = _canceledMessageIds.find(kv.first) != _canceledMessageIds.end();
            lock.unlock();
            if (isExist) continue;
            //执行回调
            kv.second();
        }
        lock.lock();
        //如果解锁期间已经notify过了，不需要wait
        if (_hasPost) {
            _hasPost = false;
            continue;
        }
        //duration为0说明没有更多消息了
        if (duration == std::chrono::milliseconds(0)) {
            _cv.wait(lock);
        }else {
            _cv.wait_until(lock, now+duration);
        }
    }
}

uint32_t MessageQueue::PostMessage(int afterSec, MessageCallback callback) {
    std::lock_guard<std::mutex> lock(_mutex);
    //计算消息发生的时间点
    TimePoint timePoint = std::chrono::system_clock::now() + std::chrono::seconds(afterSec);
    uint32_t messageId = _currentMsgId;
    Message *newMessage = new Message{
        callback,
        messageId,
        timePoint,
        NULL,
    };
    //维护消息id
    _currentMsgId++;
    if (_currentMsgId == UINT32_MAX) {
        _currentMsgId = 1;
    }
    Message *lastMessage = NULL;
    Message *currentMessage = _firstMessage;
    //找到消息在链表中应该处于的位置
    while (currentMessage != NULL && currentMessage->_when < newMessage->_when) {
        lastMessage = currentMessage;
        currentMessage = currentMessage->_next;
    }
    //如果链表是空的
    if (lastMessage == NULL) {
        //将链表接到新消息后面
        newMessage->_next = _firstMessage;
        //新消息赋值到链表头
        _firstMessage = newMessage;
    }
    //如果链表不是空的，插入
    else {
        //如果到了链表尾
        if (currentMessage == NULL) {
            lastMessage->_next = newMessage;
        }else {
            lastMessage->_next = newMessage;
            newMessage->_next = currentMessage;
        }
    }
    //唤醒runloop
    _cv.notify_all();
    _hasPost = true;
    return messageId;
}

bool MessageQueue::CancelMessage(uint32_t messageId) {
    std::lock_guard<std::mutex> lock(_mutex);
    Message *lastMessage = NULL;
    Message *currentMessage = _firstMessage;
    while (currentMessage != NULL) {
        //如果找到了该消息
        if (currentMessage->_messageId == messageId) {
            break;
        }
        lastMessage = currentMessage;
        currentMessage = currentMessage->_next;
    }
    //加入到已取消的数组（即时没找到也要加入，用于判断callback是否执行）
    _canceledMessageIds[messageId] = true;
    //没找到
    if (currentMessage == NULL) {
        return false;
    }
    //将消息从链表中移除
    if (lastMessage == NULL) {
        _firstMessage = currentMessage->_next;
    }else {
        lastMessage->_next = currentMessage->_next;
    }
    //删除消息
    delete currentMessage;
    return true;
}

//仅供调试使用
std::string MessageQueue::PrintMessageQueue() {
    std::string printMsg = "MessageQueue:";
    Message *currentMessage = _firstMessage;
    while (currentMessage != NULL) {
        printMsg += std::to_string(currentMessage->_messageId) + " ";
        currentMessage = currentMessage->_next;
    }
    return printMsg;
}
