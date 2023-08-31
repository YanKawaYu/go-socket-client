//
//  tcp_error.hpp
//  go-socket-client
//
//  Copyright © 2023 Yan. All rights reserved.
//

#ifndef tcp_error_hpp
#define tcp_error_hpp

#include <stdio.h>
#include <exception>
#include <string>

class TcpException {
public:
    enum TcpExcType {
        TcpExcBadMsgType = 1,
        TcpExcBadLenEncoding = 2,
        TcpExcBadReturnCode,
        TcpExcDataExceedsPacket,
        TcpExcMsgTooLong,
        TcpExcInvalidFlag,
        TcpExcInvalidProName,
        TcpExcInvalidProVersion,
        TcpExcInvalidSocket,
        TcpExcSocketClose,
        TcpExcSocketError,
        TcpExcUngzipError,
        TcpExcGzipError,
    };
private:
    TcpExcType _excType;
    int _errorNo;
public:
    TcpException(TcpExcType excType, int errorNo = 0);
    const TcpExcType GetExcType() const { return _excType; }
    const int GetErrno() const { return _errorNo; }
    std::string What();
};

#endif /* tcp_error_hpp */
