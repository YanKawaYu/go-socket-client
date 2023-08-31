//
//  message_queue.hpp
//  go-socket-client
//
//  Copyright © 2023 Yan. All rights reserved.
//

#ifndef message_queue_hpp
#define message_queue_hpp

#include <stdio.h>
#include <functional>
#include <map>
#include <condition_variable>
#include <mutex>
#include <chrono>

typedef std::function<void()> MessageCallback;
typedef std::chrono::system_clock::time_point TimePoint;

struct Message {
    MessageCallback _callback;
    uint32_t _messageId;
    TimePoint _when;
    Message* _next;
};

class MessageQueue {
private:
    static MessageQueue *_instance;
    Message *_firstMessage;
    std::mutex _mutex;
    std::condition_variable _cv;
    uint32_t _currentMsgId;
    bool _hasPost; //用于处理notify在wait之前的情况
    std::map<uint32_t, bool> _canceledMessageIds;

public:
    static MessageQueue* SharedQueue();

    uint32_t PostMessage(int afterSec, MessageCallback callback);
    bool CancelMessage(uint32_t messageId);
    //测试用
    std::string PrintMessageQueue();
private:
    MessageQueue();

    void __RunLoop();
};

#endif /* message_queue_hpp */
