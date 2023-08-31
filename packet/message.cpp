//
//  message.cpp
//  go-socket-client
//
//  Copyright © 2023 Yan. All rights reserved.
//

#include "message.hpp"
#include <iostream>
#include "log/zlogger.hpp"

Connect::Connect() {
    _header.SetMsgType(kMsgConnect);
}

void Connect::Encode(IWriter *writer) {
    std::vector<char> buf;
    //协议名
    setString(_protocolName, &buf);
    //协议版本
    setUint8(_protocolVersion, &buf);
    //标志位，预留
    uint8_t flags = _enableGzip << 7;
    setUint8(flags, &buf);
    //连接时间
    setUint16(_keepAlive, &buf);
    //初始化载荷
    std::vector<char> payloadBuf;
    if (kEnableGzip) {
        setGzipString(_payload, &payloadBuf);
    }else {
        setString(_payload, &payloadBuf);
    }
    //将头部写入socket
    std::vector<char> finalBuf;
    WriteHeader(finalBuf, &buf, int32_t(payloadBuf.size()));
    //将载荷写入socket
    std::copy(payloadBuf.begin(), payloadBuf.end(), std::back_inserter(finalBuf));
    writer->Write(finalBuf.data(), finalBuf.size());
}

void Connect::Decode(IReader *reader) {
    //剩余长度
    uint32_t remainLen = _header.GetRemainLen();
    //协议名
    _protocolName = getString(reader, remainLen);
    if (_protocolName != kProtocolName) {
        zcerror2("invalid protocol name:%s", _protocolName.c_str());
        throw TcpException(TcpException::TcpExcInvalidProName);
    }
    //协议版本号
    _protocolVersion = getUint8(reader, remainLen);
    if (_protocolVersion > kProtocolVersion) {
        zcerror2("invalid protocol version:%d", _protocolVersion);
        throw TcpException(TcpException::TcpExcInvalidProVersion);
    }
    //标志位，暂不使用
    uint8_t flags = getUint8(reader, remainLen);
    _enableGzip = (flags & 0x80)>0;
    if (flags != 0 && flags != 128) {
        zcerror2("invalid flag:%d", flags);
        throw TcpException(TcpException::TcpExcInvalidFlag);
    }
    //保持连接时间
    _keepAlive = getUint16(reader, remainLen);
    //内容
    if (kEnableGzip) {
        _payload = getGzipString(reader, remainLen);
    }else {
        _payload = getString(reader, remainLen);
    }

    if (remainLen != 0) {
        zcerror2("message too long, remain len:%d", remainLen);
        throw TcpException(TcpException::TcpExcMsgTooLong);
    }
}

ConnAck::ConnAck() {
    _header.SetMsgType(kMsgConnect);
}

void ConnAck::Encode(IWriter *writer) {
    std::vector<char> buf;
    //标志位，暂不使用
    setUint8(0, &buf);
    //返回码
    setUint8(uint8_t(_returnCode), &buf);
    //将头部写入socket
    std::vector<char> finalBuf;
    WriteHeader(finalBuf, &buf, 0);
    writer->Write(finalBuf.data(), finalBuf.size());
}

void ConnAck::Decode(IReader *reader) {
    uint32_t remainLen = _header.GetRemainLen();
    uint8_t flags = getUint8(reader, remainLen);
    if (flags != 0) {
        throw TcpException(TcpException::TcpExcInvalidFlag);
    }
    uint8_t retCode = getUint8(reader, remainLen);
    if (retCode >= RetCodeInvalid || retCode < 0) {
        throw TcpException(TcpException::TcpExcBadReturnCode);
    }
    _returnCode = ReturnCode(retCode);
    if (remainLen != 0) {
        zcerror2("message too long, remain len:%d", remainLen);
        throw TcpException(TcpException::TcpExcMsgTooLong);
    }
}

PingReq::PingReq() {
    _header.SetMsgType(kMsgPingReq);
}

void PingReq::Encode(IWriter *writer) {
    //将头部写入socket
    std::vector<char> finalBuf;
    WriteHeader(finalBuf, nullptr, 0);
    writer->Write(finalBuf.data(), finalBuf.size());
}

void PingReq::Decode(IReader *reader) {
    uint32_t remainLen = _header.GetRemainLen();
    if (remainLen != 0) {
        zcerror2("message too long, remain len:%d", remainLen);
        throw TcpException(TcpException::TcpExcMsgTooLong);
    }
}

PingResp::PingResp() {
    _header.SetMsgType(kMsgPingResp);
}

void PingResp::Encode(IWriter *writer) {
    //将头部写入socket
    std::vector<char> finalBuf;
    WriteHeader(finalBuf, nullptr, 0);
    writer->Write(finalBuf.data(), finalBuf.size());
}

void PingResp::Decode(IReader *reader) {
    uint32_t remainLen = _header.GetRemainLen();
    if (remainLen != 0) {
        zcerror2("message too long, remain len:%d", remainLen);
        throw TcpException(TcpException::TcpExcMsgTooLong);
    }
}

Disconnect::Disconnect() {
    _header.SetMsgType(kMsgDisconnect);
    _type = DiscTypeNone;
}

void Disconnect::Encode(IWriter *writer) {
    std::vector<char> buf;
    //类型
    setUint8(uint8_t(_type), &buf);

    std::vector<char> finalBuf;
    WriteHeader(finalBuf, &buf, 0);
    writer->Write(finalBuf.data(), finalBuf.size());
}

void Disconnect::Decode(IReader *reader) {
    uint32_t remainLen = _header.GetRemainLen();
    //类型
    _type = DisconnectType(getUint8(reader, remainLen));
    if (remainLen != 0) {
        zcerror2("message too long, remain len:%d", remainLen);
        throw TcpException(TcpException::TcpExcMsgTooLong);
    }
}

SendReq::SendReq() {
    _header.SetMsgType(kMsgSendReq);
}

void SendReq::SetData(char *data, int size) {
    _hasData = true;
    _data = std::vector<char>(data, data+size);
}

void SendReq::Encode(IWriter *writer) {
    //标志位
    uint8_t flags = (uint8_t)((_replyLevel << 1) | (_hasData << 3));
    _header.SetFlags(flags);

    std::vector<char> buf;
    //消息id
    setUint16(_messageId, &buf);
    //消息类型
    setString(_type, &buf);
    //初始化载荷
    std::vector<char> payloadBuf;
    if (kEnableGzip) {
        setGzipString(_payload, &payloadBuf);
    }else {
        setString(_payload, &payloadBuf);
    }
    //数据
    std::vector<char> dataBuf;
    if (_hasData) {
        if (kEnableGzip) {
            setGzipData(_data, &dataBuf);
        }else {
            setData(_data, &dataBuf);
        }
    }
    //写入头部
    std::vector<char> finalBuf;
    WriteHeader(finalBuf, &buf, int32_t(payloadBuf.size()), int32_t(dataBuf.size()));
    //写入载荷和二进制数据
    std::copy(payloadBuf.begin(), payloadBuf.end(), std::back_inserter(finalBuf));
    std::copy(dataBuf.begin(), dataBuf.end(), std::back_inserter(finalBuf));
    writer->Write(finalBuf.data(), finalBuf.size());
}

void SendReq::Decode(IReader *reader) {
    //回复等级
    _replyLevel = ReplyLevel((_header.GetFlags() & 0x06) >> 1);
    //是否有二进制数据
    _hasData = (_header.GetFlags() & 0x08) >> 3;
    //剩余长度
    uint32_t remainLen = _header.GetRemainLen();
    //消息id
    _messageId = getUint16(reader, remainLen);
    //消息类型
    _type = getString(reader, remainLen);
    //载荷
    if (kEnableGzip) {
        _payload = getGzipString(reader, remainLen);
    }else {
        _payload = getString(reader, remainLen);
    }
    //如果有二进制数据
    if (_hasData) {
        if (kEnableGzip) {
            _data = getGzipData(reader, remainLen);
        }else {
            _data = getData(reader, remainLen);
        }
    }

    if (remainLen != 0) {
        zcerror2("message too long, remain len:%d", remainLen);
        throw TcpException(TcpException::TcpExcMsgTooLong);
    }
}

SendResp::SendResp() {
    _header.SetMsgType(kMsgSendResp);
}

void SendResp::Encode(IWriter *writer) {
    std::vector<char> buf, payloadBuf;
    //消息id
    setUint16(_messageId, &buf);
    //初始化载荷
    if (kEnableGzip) {
        setGzipString(_payload, &payloadBuf);
    }else {
        setString(_payload, &payloadBuf);
    }
    //写入头部
    std::vector<char> finalBuf;
    WriteHeader(finalBuf, &buf, int32_t(payloadBuf.size()));
    //写入载荷
    std::copy(payloadBuf.begin(), payloadBuf.end(), std::back_inserter(finalBuf));
    writer->Write(payloadBuf.data(), payloadBuf.size());
}

void SendResp::Decode(IReader *reader) {
    //剩余长度
    uint32_t remainLen = _header.GetRemainLen();
    //消息id
    _messageId = getUint16(reader, remainLen);
    //内容
    if (kEnableGzip) {
        _payload = getGzipString(reader, remainLen);
    }else {
        _payload = getString(reader, remainLen);
    }
    if (remainLen != 0) {
        zcerror2("message too long, remain len:%d", remainLen);
        throw TcpException(TcpException::TcpExcMsgTooLong);
    }
}
