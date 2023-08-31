//
//  encoding.hpp
//  go-socket-client
//
//  Copyright © 2023 Yan. All rights reserved.
//

#ifndef encoding_hpp
#define encoding_hpp

#include <stdio.h>
#include <tuple>
#include <string>
#include <vector>
#include "tcp_io.hpp"
#include "tcp_error.hpp"

std::tuple<uint32_t, int> decodeLength(IReader *reader);
void encodeLength(uint32_t length, std::vector<char> *buf);

uint8_t getUint8(IReader *reader, uint32_t &packetRemaining);
void setUint8(uint8_t val, std::vector<char> *buf);

uint16_t getUint16(IReader *reader, uint32_t &packetRemaining);
void setUint16(uint16_t val, std::vector<char> *buf);

std::string getString(IReader *reader, uint32_t &packetRemaining);
void setString(std::string val, std::vector<char> *buf);

std::string getGzipString(IReader *reader, uint32_t &packetRemaining);
void setGzipString(std::string val, std::vector<char> *buf);

std::vector<char> getData(IReader *reader, uint32_t &packetRemaining);
void setData(std::vector<char> data, std::vector<char> *buf);

std::vector<char> getGzipData(IReader *reader, uint32_t &packetRemaining);
void setGzipData(std::vector<char> data, std::vector<char> *buf);

bool __gzipCompress(const char *src, int srcLen, std::vector<char> &output);
bool __gzipUncompress(const char *src, int srcLen, std::vector<char> &output);

#endif /* encoding_hpp */
