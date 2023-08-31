//
//  message_base.cpp
//  go-socket-client
//
//  Copyright © 2023 Yan. All rights reserved.
//

#include "message_base.hpp"
#include <iostream>

MessageBase::~MessageBase() {}

void MessageBase::WriteHeader(std::vector<char> &headerBuf, std::vector<char> *mutableHeader, int32_t payloadLen, int32_t dataLen) {
    int64_t mutableHeaderLen;
    //可变头部是否为空
    if (mutableHeader != nullptr) {
        mutableHeaderLen = mutableHeader->size();
    }else {
        mutableHeaderLen = 0;
    }
    //计算报文剩余总长度（可变报头+有效载荷）
    int64_t totalPayloadLen = mutableHeaderLen + payloadLen + dataLen;
    //如果报文剩余总长度大于最大长度
    if (totalPayloadLen > kMaxPayloadSize) {
        throw TcpException(TcpException::TcpExcMsgTooLong);
    }
    _header.SetRemainLen(int32_t(totalPayloadLen));
    //向缓冲写入消息固定头部
    _header.Encode(&headerBuf);
    //如果有可变头部，向缓冲写入消息可变头部
    if (mutableHeader != nullptr) {
        headerBuf.insert(headerBuf.end(), mutableHeader->begin(), mutableHeader->end());
    }
}
