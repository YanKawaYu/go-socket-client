//
//  ssl_connector.hpp
//  go-socket-client
//
//  Copyright © 2023 Yan. All rights reserved.
//

#ifndef ssl_connector_hpp
#define ssl_connector_hpp

#include <stdio.h>
#include "tcp_connector.hpp"

class SSLConnector : public TcpConnector {
public:
    SSLConnector(const AddrInfo *addr, unsigned int index, unsigned int connTimeout);

    void PreSelect(SocketSelect& sel);
    void AfterSelect(SocketSelect& sel);
private:
    TcpSocket * __NewSocket(SOCKET socket);

    void PreSSLSelect(SocketSelect& sel);
    void AfterSSLSelect(SocketSelect& sel);
};

#endif /* ssl_connector_hpp */
