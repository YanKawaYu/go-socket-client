//
//  encoding.cpp
//  go-socket-client
//
//  Copyright © 2023 Yan. All rights reserved.
//

#include "encoding.hpp"
#include <iostream>
#include <string.h>
#include <zlib.h>
#include <vector>

std::tuple<uint32_t, int> decodeLength(IReader *reader) {
    int32_t v = 0;
    uint8_t buf[1];
    uint shift = 0;
    for (int i=0; i<4; i++) {
        reader->ReadFull(buf, 1);
        char b = buf[0];
        v |= int32_t(b&0x7f) << shift;
        if ((b&0x80) == 0) {
            return std::make_tuple(v, i+1);
        }
        shift += 7;
    }
    return std::make_tuple(-1, -1);
}

void encodeLength(uint32_t length, std::vector<char> *buf) {
    if (length == 0) {
        buf->push_back(0);
    }
    while (length > 0) {
        uint8_t digit = length & 0x7f;
        length = length >> 7;
        if (length > 0) {
            digit |= 0x80;
        }
        buf->push_back(digit);
    }
}

uint8_t getUint8(IReader *reader, uint32_t &packetRemaining) {
    if (packetRemaining < 1) {
        throw TcpException(TcpException::TcpExcDataExceedsPacket);
    }
    char buf[1];
    reader->ReadFull(buf, 1);
    packetRemaining--;
    return buf[0];
}

void setUint8(uint8_t val, std::vector<char> *buf) {
    buf->push_back(val);
}

uint16_t getUint16(IReader *reader, uint32_t &packetRemaining) {
    if (packetRemaining < 2) {
        throw TcpException(TcpException::TcpExcDataExceedsPacket);
    }
    //有些系统的char可能是有符号的，当无符号强转有符号会发生补位导致数据出错，故这里用uint8
    uint8_t buf[2];
    reader->ReadFull(buf, 2);
    packetRemaining -= 2;
    return (uint16_t(buf[0])<<8) | uint16_t(buf[1]);
}

void setUint16(uint16_t val, std::vector<char> *buf) {
    buf->push_back((val & 0xff00) >> 8);
    buf->push_back(val & 0x00ff);
}

std::string getString(IReader *reader, uint32_t &packetRemaining) {
    int32_t strLen;
    int lenLen;
    std::tie(strLen, lenLen) = decodeLength(reader);
    //减去长度所占的字节数
    packetRemaining -= lenLen;

    if (packetRemaining < strLen) {
        throw TcpException(TcpException::TcpExcDataExceedsPacket);
    }
    char buf[strLen];
    reader->ReadFull(buf, strLen);
    packetRemaining -= strLen;

    return std::string(buf, buf+strLen);
}

void setString(std::string val, std::vector<char> *buf) {
    int32_t length = int32_t(val.length());
    encodeLength(length, buf);
    std::copy(val.begin(), val.end(), std::back_inserter(*buf));
}

std::string getGzipString(IReader *reader, uint32_t &packetRemaining) {
    int32_t gzipLen;
    int lenLen;
    std::tie(gzipLen, lenLen) = decodeLength(reader);
    //减去长度所占的字节数
    packetRemaining -= lenLen;

    if (packetRemaining < gzipLen) {
        throw TcpException(TcpException::TcpExcDataExceedsPacket);
    }

    char buf[gzipLen];
    reader->ReadFull(buf, gzipLen);
    packetRemaining -= gzipLen;

    std::string result = "";
    if (gzipLen > 0) {
        std::vector<char> output;
        if (!__gzipUncompress(buf, gzipLen, output)) {
            throw TcpException(TcpException::TcpExcUngzipError);
        }
        result = std::string(output.begin(), output.end());
    }
    return result;
}

void setGzipString(std::string val, std::vector<char> *buf) {
    std::vector<char> output;
    if (!__gzipCompress(val.c_str(), (int)val.size(), output)) {
        throw TcpException(TcpException::TcpExcGzipError);
    }
    uint32_t length = (uint32_t)output.size();
    encodeLength(length, buf);
    std::copy(output.begin(), output.end(), std::back_inserter(*buf));
}

std::vector<char> getData(IReader *reader, uint32_t &packetRemaining) {
    int32_t dataLen;
    int lenLen;
    std::tie(dataLen, lenLen) = decodeLength(reader);
    //减去长度所占的字节数
    packetRemaining -= lenLen;

    if (packetRemaining < dataLen) {
        throw TcpException(TcpException::TcpExcDataExceedsPacket);
    }

    char buf[dataLen];
    reader->ReadFull(buf, dataLen);
    packetRemaining -= dataLen;
    return std::vector<char>(buf, buf+dataLen);
}

void setData(std::vector<char> data, std::vector<char> *buf) {
    int32_t length = int32_t(data.size());
    encodeLength(length, buf);
    std::copy(data.begin(), data.end(), std::back_inserter(*buf));
}

std::vector<char> getGzipData(IReader *reader, uint32_t &packetRemaining) {
    int32_t gzipLen;
    int lenLen;
    std::tie(gzipLen, lenLen) = decodeLength(reader);
    //减去长度所占的字节数
    packetRemaining -= lenLen;

    if (packetRemaining < gzipLen) {
        throw TcpException(TcpException::TcpExcDataExceedsPacket);
    }

    char buf[gzipLen];
    reader->ReadFull(buf, gzipLen);
    packetRemaining -= gzipLen;

    std::vector<char> output;
    if (gzipLen > 0) {
        if (!__gzipUncompress(buf, gzipLen, output)) {
            throw TcpException(TcpException::TcpExcUngzipError);
        }
    }
    return output;
}

void setGzipData(std::vector<char> data, std::vector<char> *buf) {
    if (data.size() == 0) return;
    std::vector<char> output;
    if (!__gzipCompress(&data[0], (int)data.size(), output)) {
        throw TcpException(TcpException::TcpExcGzipError);
    }
    uint32_t length = (uint32_t)output.size();
    encodeLength(length, buf);
    std::copy(output.begin(), output.end(), std::back_inserter(*buf));
}

bool __gzipCompress(const char *src, int srcLen, std::vector<char> &output) {
    z_stream stream;
    stream.zalloc = Z_NULL;
    stream.zfree = Z_NULL;
    stream.opaque = Z_NULL;
    stream.avail_in = (uint)srcLen;
    stream.next_in = (Bytef *)(void *)src;
    stream.total_out = 0;
    stream.avail_out = 0;

    static const uint32_t ChunkSize = 16384;

    int compression = Z_DEFAULT_COMPRESSION;
    if (deflateInit2(&stream, compression, Z_DEFLATED, 31, 8, Z_DEFAULT_STRATEGY) == Z_OK)
    {
        char outBuff[ChunkSize];
        while (stream.avail_out == 0)
        {
            stream.next_out = (uint8_t *)outBuff;
            stream.avail_out = (uInt)ChunkSize;
            deflate(&stream, Z_FINISH);
            output.insert(output.end(), outBuff, outBuff+(stream.total_out-output.size()));
            memset(outBuff, 0, sizeof(outBuff));
        }
        deflateEnd(&stream);
        return true;
    }
    return false;
}

bool __gzipUncompress(const char *src, int srcLen, std::vector<char> &output) {
    z_stream stream;
    stream.zalloc = Z_NULL;
    stream.zfree = Z_NULL;
    stream.avail_in = (uint)srcLen;
    stream.next_in = (Bytef *)src;
    stream.total_out = 0;
    stream.avail_out = 0;

    if (inflateInit2(&stream, 47) == Z_OK)
    {
        int status = Z_OK;
        char outBuff[srcLen*2];
        while (status == Z_OK)
        {
            stream.next_out = (uint8_t *)outBuff;
            stream.avail_out = (uInt)sizeof(outBuff);
            status = inflate (&stream, Z_SYNC_FLUSH);
            output.insert(output.end(), outBuff, outBuff+(sizeof(outBuff)-stream.avail_out));
            memset(outBuff, 0, sizeof(outBuff));
        }
        if (inflateEnd(&stream) == Z_OK)
        {
            if (status == Z_STREAM_END)
            {
                return true;
            }
        }
    }
    return false;
}
