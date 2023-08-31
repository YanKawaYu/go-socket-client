//
//  message_base.h
//  go-socket-client
//
//  Copyright © 2023 Yan. All rights reserved.
//

#ifndef message_base_h
#define message_base_h

#include "fix_header.hpp"

class MessageBase {
public:
    virtual ~MessageBase();
    virtual void Encode(IWriter *writer) = 0;
    virtual void Decode(IReader *reader) = 0;
public:
    void SetHeader(FixHeader header) { _header = header; }
    const MessageType GetMsgType() const { return _header.GetMsgType(); }
protected:
    //报文最大长度
    const int kMaxPayloadSize = (1 << (4 * 7)) - 1;
    //固定包头
    FixHeader _header;

    void WriteHeader(std::vector<char> &headerBuf, std::vector<char> *mutableHeader, int32_t payloadLen, int32_t dataLen = 0);
};

#endif /* message_base_h */
