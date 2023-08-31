//
//  fix_header.hpp
//  go-socket-client
//
//  Copyright © 2023 Yan. All rights reserved.
//

#ifndef fix_header_hpp
#define fix_header_hpp

#include <stdio.h>
#include "tcp_io.hpp"
#include "tcp_error.hpp"
#include <vector>

enum MessageType {
    kMsgFirst = 0,

    kMsgConnect = 1,
    kMsgConnAck,
    kMsgPingReq,
    kMsgPingResp,
    kMsgDisconnect,
    kMsgSendReq,
    kMsgSendResp,

    kMsgLast,
};

class FixHeader {
public:
    void Encode(std::vector<char> *);
    void Decode(IReader*);

private:
    MessageType _msgType;
    uint32_t _remainLen;
    uint8_t _flags = 0;
public:
    MessageType GetMsgType() const { return _msgType;}
    uint32_t GetRemainLen() const {return _remainLen;}
    uint8_t GetFlags() const {return _flags;}
    void SetMsgType(MessageType msgType) { _msgType = msgType; }
    void SetRemainLen(uint32_t remainLen) { _remainLen = remainLen; }
    void SetFlags(uint8_t flags) { _flags = flags;}
};

#endif /* fix_header_hpp */
