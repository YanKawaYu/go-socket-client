//
//  dns_query.cpp
//  go-socket-client
//
//  Copyright © 2023 Yan. All rights reserved.
//

#include "dns_query.hpp"
#include <netdb.h>
#include "../packet/unix_socket.hpp"
#include "log/zlogger.hpp"
#include <errno.h>
#include <thread>
#include <cstring>
#include "common/time_utils.h"

DnsQuery* DnsQuery::_instance = NULL;

DnsQuery* DnsQuery::SharedQuery() {
    static std::once_flag onceFlag;
    std::call_once(onceFlag, []{
        DnsQuery::_instance = new DnsQuery();
    });
    return DnsQuery::_instance;
}

DnsQuery::DnsQuery() {}

void DnsQuery::__GetIps() {
    struct addrinfo hints, *result = NULL;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_flags = AI_PASSIVE;
    hints.ai_socktype = SOCK_STREAM;
    int ret = getaddrinfo(_hostName.c_str(), NULL, &hints, &result);
    if (ret != 0) {
        zcwarn2("getaddrinfo error %d(%s)", errno, strerror(errno));
    }else {
        _cacheIps.clear();
        for (struct addrinfo *cur = result; cur != NULL; cur = cur->ai_next) {
            struct sockaddr_in *addr = (struct sockaddr_in *)cur->ai_addr;
            std::string ip = std::string(inet_ntoa(addr->sin_addr));
            _cacheIps.push_back(ip);
        }
    }
    if (result != NULL) {
        freeaddrinfo(result);
    }
    //通知DNS查询结束
    _retBreaker.Break();
}

bool DnsQuery::GetHostByName(const std::string hostName, std::string backupIp, std::vector<std::string> &ips, SocketBreaker &breaker, unsigned int timeout) {
    uint64_t currentTick = gettickcount();
    //优先httpdns
    std::vector<std::string> httpDns;
    //If there is a local dns querier
    if (_getHttpDns != NULL) {
        httpDns = _getHttpDns(hostName);
    }
    uint64_t httpDnsDuration = gettickcount()-currentTick;
    //当httpdns为空时走正常dns查询
    if (httpDns.empty()) {
        if (!_retBreaker.Clear()) {
            _retBreaker.Close();
            _retBreaker.ReCreate();
        }

        _hostName = hostName;
        //启动DNS查询线程
        std::thread queryThread(&DnsQuery::__GetIps, this);
        queryThread.detach();
        //DNS超时
        SocketSelect sel(breaker);
        sel.PreSelect();
        sel.ExceptionFdSet(_retBreaker.BreakerFd());
        sel.ReadFdSet(_retBreaker.BreakerFd());
        int selRet = sel.Select(timeout);
        //如果断开连接，直接返回
        if (sel.IsBreak()) {
            return false;
        }
        //注意，这里不管是超时还是出错，都可以返回内存缓存的ip
        //如果超时
        if (selRet == 0) {
            zcwarn2("dns timeout");
        }
        if (selRet < 0) {
            zcwarn2("selRet < 0, errno:%d", sel.GetErrno());
        }
        if (sel.IsException()) {
            zcwarn2("sel exception, errno:%d(%s)", errno, strerror(errno));
        }
        if (sel.ExceptionFdIsSet(_retBreaker.BreakerFd())) {
            zcwarn2("ret breaker exception");
        }
        //返回结果
        ips.insert(ips.end(), _cacheIps.begin(), _cacheIps.end());
    } else {
        ips.insert(ips.end(), httpDns.begin(), httpDns.end());
    }

    //如果没有解析到域名，使用硬编码的备用ip
    if (ips.empty()) {
        ips.push_back(backupIp);
        zcinfo2("dns unavailable, use backup ip %s, httpDns duration: %d ms, total duration: %d ms", backupIp.c_str(), httpDnsDuration, gettickcount()-currentTick);
    } else {
        std::string allIp = "";
        for (std::string ip : ips) {
            allIp += ip + ",";
        }
        zcinfo2("finish dns, found %s httpDns duration: %d ms, total duration: %d ms", allIp.c_str(), httpDnsDuration, gettickcount()-currentTick);
    }

    return true;
}
