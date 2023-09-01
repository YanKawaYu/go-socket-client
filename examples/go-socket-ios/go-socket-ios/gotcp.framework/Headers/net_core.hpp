//
//  net_core.hpp
//  go-socket-client
//
//  Copyright © 2023 Yan. All rights reserved.
//

#ifndef net_core_hpp
#define net_core_hpp

#include <stdio.h>
#include <string>
#include "tcp_define.h"

class NetCore {
public:
    static void InitHostAndPort(std::string host, std::string backupIp, int port, bool isTls, GetConnectInfo getConnectInfo);
    static void MakeSureConnected();
    static void DisconnectAsync(DisconnectCode code, bool shouldReconnect);
    static bool Send(std::string, std::string, OnSendRespCallback);
    static bool Send(std::string, std::string, char *, int, OnSendRespCallback);
    static bool SendNoReply(std::string, std::string);

    static void SetOnServerDisconnect(OnServerDisconnect onServerDisconnect);
    static void SetOnConnStatusChange(OnConnStatusChange onStatusChange);
    //提供给前端用于HTTPDNS的回调
    static void SetHttpDns(GetHttpDns getHttpDns);
    static void SetOnSendReq(OnSendReqCallback callback);
    static void SetNetType(NetType netType);

    static ConnectStatus GetConnectStatus();
};

#endif /* net_core_hpp */
