//
//  tcp_connector.hpp
//  go-socket-client
//
//  Copyright © 2023 Yan. All rights reserved.
//

#ifndef tcp_connector_hpp
#define tcp_connector_hpp

#include <stdio.h>
#include <string>
#include "../packet/unix_socket.hpp"
#include "../socket/socket_select.hpp"
#include "../socket/tcp_socket.hpp"

typedef struct AddrInfo{
    std::string ip;
    uint16_t port;
    bool isTls;
}AddrInfo;

typedef enum {
    ConnStatusStart,
    ConnStatusConnecting,
    ConnStatusError,
    ConnStatusStartSSL, //SSL开始连接，TCP连接完成，SSL握手未完成的状态
    ConnStatusSSLConnecting, //SSL连接中
    ConnStatusConnected,
    ConnStatusEnd,
}ConnectorStatus;

class TcpConnector {
public:
    TcpConnector(const AddrInfo *addr, unsigned int index, unsigned int connTimeout);
    ~TcpConnector();
    TcpSocket * Socket() const { return _socket; };
    void SetSocket(TcpSocket *socket) { _socket = socket; };
    int Timeout() const;
    ConnectorStatus Status() { return _status; };
    void Close();

    virtual void PreSelect(SocketSelect& sel);
    virtual void AfterSelect(SocketSelect& sel);
protected:
    const AddrInfo *_addr;
    TcpSocket *_socket;
    ConnectorStatus _status;
    unsigned int _index;
    unsigned int _connTimeout;
    uint64_t _startConnTime;
    uint64_t _endConnTime;

    virtual TcpSocket * __NewSocket(SOCKET socket);

    void PreConnectSelect(SocketSelect& sel);
    void AfterConnectSelect(SocketSelect& sel);
};

#endif /* tcp_connector_hpp */
