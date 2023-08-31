//
//  complex_connect.hpp
//  go-socket-client
//
//  Copyright © 2023 Yan. All rights reserved.
//

#ifndef complex_connect_hpp
#define complex_connect_hpp

#include <stdio.h>
#include "../packet/unix_socket.hpp"
#include "../socket/socket_breaker.hpp"
#include <vector>
#ifdef WITH_SSL
#include "ssl_connector.hpp"
#else
#include "tcp_connector.hpp"
#endif
#include "../socket/tcp_socket.hpp"

class ComplexConnect {
public:
    ComplexConnect(unsigned int timeout, unsigned int interval,
                   unsigned int errorInterval, unsigned int maxConnect);

    TcpSocket* ConnectImpatient(const std::vector<AddrInfo>& addrs, SocketBreaker &breaker);
private:
    unsigned int _timeout; //ms
    unsigned int _interval; //ms
    unsigned int _errorInterval; //ms
    unsigned int _maxConnect;
};

#endif /* complex_connect_hpp */
