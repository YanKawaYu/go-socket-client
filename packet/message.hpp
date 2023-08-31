//
//  message.hpp
//  go-socket-client
//
//  Copyright © 2023 Yan. All rights reserved.
//

#ifndef message_hpp
#define message_hpp

#include <stdio.h>
#include <string>
#include "tcp_io.hpp"
#include "message_base.hpp"
#include "encoding.hpp"

//Protocol name, must be the same as go-socket server
//Otherwise the server will close the connection immediately
//协议名称
#define kProtocolName "GOSOC"
//Protocol version, in case that the protocol upgrades in the future
//协议版本号
#define kProtocolVersion 1
#define kEnableGzip true

/**
 *  连接
 */
class Connect : public MessageBase {
public:
    Connect();
    void Encode(IWriter *writer) override;
    void Decode(IReader *reader) override;
public:
    void SetPayload(std::string payload) { _payload = payload; };
    void SetProtocolName(std::string protocolName) { _protocolName = protocolName; };
    void SetKeepAlive(uint16_t keepAlive) { _keepAlive = keepAlive; };
    void SetProtocolVersion(uint8_t protocolVersion) { _protocolVersion = protocolVersion; };
    void SetEnableGzip(bool enableGzip) { _enableGzip = enableGzip; };
private:
    std::string _protocolName;
    uint8_t _protocolVersion;
    uint16_t _keepAlive;
    std::string _payload;
    bool _enableGzip;
};

/**
 *  连接回复
 */
class ConnAck : public MessageBase {
public:
    enum ReturnCode {
        RetCodeAccepted = 0,
        RetCodeServerUnavailable = 1,
        RetCodeBadLoginInfo,
        RetCodeNotAuthorized,
        RetCodeAlreadyConnected,
        RetCodeConcurrentLogin,
        RetCodeBadToken,

        RetCodeInvalid,
    };
public:
    ConnAck();
    void Encode(IWriter *writer) override;
    void Decode(IReader *reader) override;
public:
    ReturnCode GetReturnCode() { return _returnCode; };
private:
    ReturnCode _returnCode;
};

/**
 *  心跳包
 */
class PingReq : public MessageBase {
public:
    PingReq();
    void Encode(IWriter *writer) override;
    void Decode(IReader *reader) override;
};

/*
 *  心跳回应包
 */
class PingResp : public MessageBase {
public:
    PingResp();
    void Encode(IWriter *writer) override;
    void Decode(IReader *reader) override;
};

/*
 *  断开连接
 */
class Disconnect : public MessageBase {
public:
    enum DisconnectType {
        DiscTypeNone = 0,
        DiscTypeKickout = 1, //踢出登录，服务器发给客户端，客户端应立即注销
    };
public:
    Disconnect();
    void Encode(IWriter *writer) override;
    void Decode(IReader *reader) override;
    DisconnectType GetDisconnectType() { return _type; };
private:
    DisconnectType _type;
};

/*
 *   发送消息
 */
class SendReq : public MessageBase {
public:
    enum ReplyLevel {
        RLevelNoReply = 0,
        RLevelReplyLater = 1, //业务逻辑返回后回复
        RLevelReplyNow, //立刻回复（业务逻辑之前）
    };
public:
    SendReq();
    void Encode(IWriter *writer) override;
    void Decode(IReader *reader) override;
public:
    uint16_t GetMessageId() { return _messageId; };
    void SetMessageId(uint16_t messageId) { _messageId = messageId; };
    void SetType(std::string type) { _type = type; };
    void SetPayload(std::string payload) { _payload = payload; };
    void SetReplyLevel(ReplyLevel replyLevel) { _replyLevel = replyLevel; };
    void SetData(char *data, int size);
    std::string GetType() { return _type; };
    std::string GetPayload() { return _payload; };
    std::vector<char> GetData() { return _data; };
private:
    ReplyLevel _replyLevel = RLevelReplyLater;
    bool _hasData = false;
    uint16_t _messageId;
    std::string _type;
    std::string _payload;
    //二进制数据
    std::vector<char> _data;
};

/*
 *  发送消息回执
 */
class SendResp : public MessageBase {
public:
    SendResp();
    void Encode(IWriter *writer) override;
    void Decode(IReader *reader) override;
public:
    const uint16_t GetMessageId() const { return _messageId; }
    const std::string GetPayload() const { return _payload; }
private:
    uint16_t _messageId;
    std::string _payload;
};

#endif /* message_hpp */
