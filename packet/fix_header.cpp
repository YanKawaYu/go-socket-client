//
//  fix_header.cpp
//  go-socket-client
//
//  Copyright © 2023 Yan. All rights reserved.
//

#include "fix_header.hpp"
#include "encoding.hpp"
#include "log/zlogger.hpp"

void FixHeader::Encode(std::vector<char> *buf) {
    if (_msgType <= kMsgFirst || _msgType >= kMsgLast) {
        throw TcpException(TcpException::TcpExcBadMsgType);
    }
    //消息类型和标志位
    uint8_t val = (_msgType << 4) | (_flags & 0x0F);
    setUint8(val, buf);
    //剩余长度
    encodeLength(_remainLen, buf);
}

void FixHeader::Decode(IReader *reader) {
    uint8_t buf[1];
    reader->ReadFull(buf, sizeof(buf));
    //消息类型
    _msgType = MessageType((buf[0] & 0xF0) >> 4);
    if (_msgType <= kMsgFirst || _msgType >= kMsgLast) {
        throw TcpException(TcpException::TcpExcBadMsgType);
    }
    //标志位
    _flags = buf[0] & 0x0F;
    if ((_flags >> 3) != 0 || (_flags & 0x01) != 0) {
        zcerror2("invalid flag:%d", _flags);
        throw TcpException(TcpException::TcpExcInvalidFlag);
    }
    //剩余长度
    int32_t remainLen;
    int len;
    std::tie(remainLen, len) = decodeLength(reader);
    _remainLen = remainLen;
}
