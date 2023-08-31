//
//  zlogger.cpp
//  go-socket-client
//
//  Copyright © 2023 Yan. All rights reserved.
//

#include "zlogger.hpp"
#include <cstring>

void zlogger2console(LogLevel level, const char* format, ...) {
    LoggerInfo info;
    memset(&info, 0, sizeof(LoggerInfo));
    info.tag = "zzconsole";
    info.level = level;

    va_list valist;
    va_start(valist, format);
    if (NULL == format) {
        info.level = kLevelFatal;
        ConsoleLog(&info, "NULL == _format");
    } else {
        char temp[4096] = {'\0'};
        vsnprintf(temp, 4096, format, valist);
        ConsoleLog(&info, temp);
    }
    va_end(valist);
}
