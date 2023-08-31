//
//  message_manager.cpp
//  go-socket-client
//
//  Copyright © 2023 Yan. All rights reserved.
//

#include "message_manager.hpp"

std::unique_ptr<MessageBase> MessageManager::DecodeMessage(IReader *reader) {
    //解析包头
    FixHeader header;
    header.Decode(reader);
    //根据消息类型构造消息
    MessageBase *message = NULL;
    switch (header.GetMsgType()) {
        case kMsgConnect:
            message = new Connect();
            break;
        case kMsgConnAck:
            message = new ConnAck();
            break;
        case kMsgDisconnect:
            message = new Disconnect();
            break;
        case kMsgPingReq:
            message = new PingReq();
            break;
        case kMsgPingResp:
            message = new PingResp();
            break;
        case kMsgSendReq:
            message = new SendReq();
            break;
        case kMsgSendResp:
            message = new SendResp();
            break;
        default:
            break;
    }
    message->SetHeader(header);
    //解析消息
    message->Decode(reader);
    return std::unique_ptr<MessageBase>(message);
}

void MessageManager::EncodeMessage(IWriter *writer, MessageBase *message) {
    message->Encode(writer);
}
