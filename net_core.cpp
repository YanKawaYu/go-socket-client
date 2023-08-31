//
//  net_core.cpp
//  go-socket-client
//
//  Copyright © 2023 Yan. All rights reserved.
//

#include "net_core.hpp"
#include "tcp_client.hpp"

static TcpClient _client;

void NetCore::InitHostAndPort(std::string host, std::string backupIp, int port, bool isTls, GetConnectInfo getConnectInfo) {
    _client.InitHostAndPort(host, backupIp, port, isTls, getConnectInfo);
}

void NetCore::MakeSureConnected() {
    _client.MakeSureConnected();
}

void NetCore::DisconnectAsync(DisconnectCode code, bool shouldReconnect) {
    _client.SendDisconnect();
    _client.DisconnectAsync(code, shouldReconnect);
}

bool NetCore::Send(std::string payloadType, std::string payload, OnSendRespCallback callback) {
    return _client.Send(payloadType, payload, callback);
}

bool NetCore::Send(std::string payloadType, std::string payload, char *data, int dataLen, OnSendRespCallback callback) {
    return _client.Send(payloadType, payload, data, dataLen, callback);
}

bool NetCore::SendNoReply(std::string payloadType, std::string payload) {
    return _client.SendNoReply(payloadType, payload);
}

void NetCore::SetOnConnStatusChange(OnConnStatusChange onStatusChange) {
    _client.SetOnConnStatusChange(onStatusChange);
}

void NetCore::SetOnServerDisconnect(OnServerDisconnect onServerDisconnect) {
    _client.SetOnServerDisconnect(onServerDisconnect);
}

void NetCore::SetHttpDns(GetHttpDns getHttpDns) {
    _client.SetHttpDns(getHttpDns);
}

void NetCore::SetOnSendReq(OnSendReqCallback callback) {
    _client.SetOnSendReq(callback);
}

void NetCore::SetNetType(NetType netType) {
    _client.SetNetType(netType);
}

ConnectStatus NetCore::GetConnectStatus() {
    return _client.GetConnectStatus();
}
