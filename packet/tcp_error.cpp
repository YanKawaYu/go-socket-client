//
//  tcp_error.cpp
//  go-socket-client
//
//  Copyright © 2023 Yan. All rights reserved.
//

#include "tcp_error.hpp"

TcpException::TcpException(TcpExcType excType, int errorNo) {
    _excType = excType;
    _errorNo = errorNo;
}

std::string TcpException::What() {
    std::string message = "unknown";
    switch (_excType) {
        case TcpExcBadMsgType:
            message = "message type error";
            break;
        case TcpExcBadLenEncoding:
            message = "bad length encoding";
            break;
        case TcpExcBadReturnCode:
            message = "bad return code";
            break;
        case TcpExcDataExceedsPacket:
            message = "data exceeds packet size limit";
            break;
        case TcpExcMsgTooLong:
            message = "message too long";
            break;
        case TcpExcInvalidFlag:
            message = "flag invalid";
            break;
        case TcpExcInvalidProName:
            message = "protocal name invalid";
            break;
        case TcpExcInvalidProVersion:
            message = "protocal version invalid";
            break;
        case TcpExcInvalidSocket:
            message = "invalid socket";
            break;
        case TcpExcSocketClose:
            message = "socket close";
            break;
        case TcpExcSocketError:
            message = "socket error:" + std::to_string(_errorNo);
            break;
        case TcpExcGzipError:
            message = "gzip error:";
            break;
        case TcpExcUngzipError:
            message = "ungzip error:";
            break;
        default:
            break;
    }
    return message;
}
