//
//  dns_query.hpp
//  go-socket-client
//
//  Copyright © 2023 Yan. All rights reserved.
//

#ifndef dns_query_hpp
#define dns_query_hpp

#include <stdio.h>
#include <string>
#include <vector>
#include "../socket/socket_select.hpp"
#include "tcp_define.h"

class DnsQuery {
public:
    static DnsQuery* SharedQuery();

    bool GetHostByName(const std::string hostName, std::string backupIp, std::vector<std::string> &ips, SocketBreaker &breaker, unsigned int timeout);
    //设置用于HTTPDNS的回调
    void SetHttpDns(GetHttpDns getHttpDns) { _getHttpDns = getHttpDns; };
private:
    DnsQuery();
    static DnsQuery *_instance;

    std::string _hostName;
    std::vector<std::string> _cacheIps;
    SocketBreaker _retBreaker;
    GetHttpDns _getHttpDns;

    void __GetIps();
};

#endif /* dns_query_hpp */
