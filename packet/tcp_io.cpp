//
//  tcp_io.cpp
//  go-socket-client
//
//  Copyright © 2023 Yan. All rights reserved.
//

#include "tcp_io.hpp"
#include <errno.h>

TcpReader::TcpReader() {
    _socket = NULL;
}

TcpReader::~TcpReader() {
    _socket = NULL;
}

void TcpReader::ReadFull(void *buf, size_t len) {
    if (_socket == NULL) {
        throw TcpException(TcpException::TcpExcInvalidSocket);
    }
    char *srcBuf = (char *)buf;
    int recvLen = 0;
    //socket读取函数一次性可能读不全，故必须循环读取
    while (recvLen < len) {
        int tmpRecvLen = _socket->Read(srcBuf, len-recvLen);
        srcBuf += tmpRecvLen;
        recvLen += tmpRecvLen;
    }
}

TcpWriter::TcpWriter() {
    _socket = NULL;
}

TcpWriter::~TcpWriter() {
    _socket = NULL;
}

void TcpWriter::Write(void *buf, size_t len) {
    if (_socket == NULL) {
        throw TcpException(TcpException::TcpExcInvalidSocket);
    }
    _socket->Write(buf, len);
}
