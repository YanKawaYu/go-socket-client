//
//  unix_socket.cpp
//  go-socket-client
//
//  Copyright © 2023 Yan. All rights reserved.
//

#include "unix_socket.hpp"

#ifdef ANDROID
#include <asm/ioctls.h>
#endif

#ifndef _WIN32

int socket_set_noblo(SOCKET fd) {
    int ret = fcntl(fd, F_GETFL, 0);
    if(ret >= 0) {
        long flags = ret | O_NONBLOCK;
        ret = fcntl(fd, F_SETFL, flags);
    }
    return ret;
}

int socket_set_blo(SOCKET fd) {
    int ret = fcntl(fd, F_GETFL, 0);
    if(ret >= 0) {
        long flags = ret & ~O_NONBLOCK;
        ret = fcntl(fd, F_SETFL, flags);
    }
    return ret;
}

#else
int socket_set_noblo(SOCKET fd) {
    static const int noblock = 1;
    return ioctlsocket(fd, FIONBIO, (u_long*)&noblock);
}
#endif

//not support in windows
int socket_set_tcp_mss(SOCKET sockfd, int size) {
#ifdef _WIN32
    return 0;
#else
    return setsockopt(sockfd, IPPROTO_TCP, TCP_MAXSEG, &size, sizeof(size));
#endif
}

int socket_get_tcp_mss(SOCKET sockfd, int* size) {
#ifdef _WIN32
    return 0;
#else
    if (size == NULL)
        return -1;
    socklen_t len = sizeof(int);
    return getsockopt(sockfd, IPPROTO_TCP, TCP_MAXSEG, (void*)size, &len);
#endif
}

int socket_fix_tcp_mss(SOCKET sockfd) {
    return socket_set_tcp_mss(sockfd, 1400);
}

int socket_disable_nagle(SOCKET sock, int nagle) {
    return setsockopt(sock, IPPROTO_TCP, TCP_NODELAY, (char *)&nagle, sizeof(nagle));
}

int socket_get_nwrite(SOCKET _sock, int* _nwriteLen) {
#if defined(__APPLE__)
    socklen_t len = sizeof(int);
    return getsockopt(_sock, SOL_SOCKET, SO_NWRITE, _nwriteLen, &len);
#elif defined(ANDROID)
    return ioctl(_sock, SIOCOUTQ, _nwriteLen);
#else
    *_nwriteLen = -1;
    return 0;
#endif
}

int socket_get_nread(SOCKET _sock, int* _nreadLen) {
#if defined(__APPLE__)
    socklen_t len = sizeof(int);
    return getsockopt(_sock, SOL_SOCKET, SO_NREAD, _nreadLen, &len);
#elif defined(ANDROID)
    return ioctl(_sock, SIOCINQ, _nreadLen);
#else
    *_nreadLen = -1;
    return 0;
#endif

}

int socket_nwrite(SOCKET _sock) {
    int value = 0;
    int ret = socket_get_nwrite(_sock, &value);

    if (0==ret) return value;
    else return ret;
}

int socket_nread(SOCKET _sock) {
    int value = 0;
    int ret = socket_get_nread(_sock, &value);

    if (0==ret) return value;
    else return ret;
}
