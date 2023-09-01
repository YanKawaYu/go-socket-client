//
//  tcp_define.h
//  go-socket-client
//
//  Copyright © 2023 Yan. All rights reserved.
//

#ifndef tcp_define_h
#define tcp_define_h

#include <functional>
#include <vector>

//修改这个枚举必须同时修改tcp_client.cpp中的map！！！
//安卓TcpClient类中的定义也必须同时修改！！！
typedef enum {
    kDisCodeNone = 0,
    kDisCodeReset = 1,
    kDisCodeNetChange,
    kDisCodeNetLost,
    kDisCodeKickout,
    kDisCodeLogout,
    kDisCodeConnectTimeout,
    kDisCodeSendReqTimeout,
    kDisCodePingTimeout,
}DisconnectCode;

typedef enum {
    kNetTypeUnknown,
    kNetTypeWifi,
    kNetTypeMobile,
    kNetTypeNoNet,
}NetType;

typedef enum {
    kConnectIdle = 0,
    kConnecting = 1,
    kConnected,
    kDisconnected,
    kConnectFailed,
    kConnectKickout,
}ConnectStatus;

typedef enum {
    kErrorCodeNone = 0, //成功
    kErrorCodeNotConnect = 1, //未连接
    kErrorCodeTimeout, //超时
    kErrorCodeLoseConnect, //连接已断开
}ErrorCode;

typedef enum {
    SvrDisNone = 0,
    SvrDisKickout = 1,
}SvrDisCode;

//发消息回调
typedef std::function<void(ErrorCode, std::string)> OnSendRespCallback;
//收推送消息回调
typedef std::function<void(std::string, std::string)> OnSendReqCallback;
//连接状态改变回调
typedef std::function<void(ConnectStatus)> OnConnStatusChange;
//踢出登陆回调
typedef std::function<void(SvrDisCode)> OnServerDisconnect;
//获取connect信息
typedef std::function<std::string()> GetConnectInfo;
//获取HttpDns
typedef std::function<std::vector<std::string>(std::string)> GetHttpDns;

#endif /* tcp_define_h */
