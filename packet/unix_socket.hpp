//
//  unix_socket.hpp
//  go-socket-client
//
//  Copyright © 2023 Yan. All rights reserved.
//

#ifndef unix_socket_hpp
#define unix_socket_hpp

#include <netinet/tcp.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <fcntl.h>

#define socket_close close

#define SOCKET int
#define INVALID_SOCKET -1

#define IS_NOBLOCK_CONNECT_ERRNO(err) ((err) == EINPROGRESS)

int socket_set_noblo(SOCKET fd);
int socket_set_blo(SOCKET fd);

int socket_set_tcp_mss(SOCKET sockfd, int size);
int socket_get_tcp_mss(SOCKET sockfd, int* size);
int socket_fix_tcp_mss(SOCKET sockfd);    // make mss=mss-40
int socket_disable_nagle(SOCKET sock, int nagle);

int socket_get_nwrite(SOCKET _sock, int* _nwriteLen);
int socket_get_nread(SOCKET _sock, int* _nreadLen);
int socket_nwrite(SOCKET _sock);
int socket_nread(SOCKET _sock);

#endif /* unix_socket_hpp */
