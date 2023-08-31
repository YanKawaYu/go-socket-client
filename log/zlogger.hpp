//
//  zlogger.hpp
//  go-socket-client
//
//  Copyright © 2023 Yan. All rights reserved.
//

#ifndef zlogger_hpp
#define zlogger_hpp

#include <stdio.h>
#include "log_define.h"
#include <stdarg.h>

void zlogger2console(LogLevel level, const char* format, ...);

#define ZLOGGER_TAG "gotcp"

#define __ZFILE__ (__FILE__)

#ifndef _MSC_VER
#define __ZFUNCTION__        __FUNCTION__
#else
// Definitely, VC6 not support this feature!
#if _MSC_VER > 1200
#define __ZFUNCTION__    __FUNCSIG__
#else
#define __ZFUNCTION__    "N/A"
#warning " is not supported by this compiler"
#endif
#endif

//for console
#define zcdebug2(...) zlogger2console(kLevelDebug, __VA_ARGS__)
#define zcinfo2(...) zlogger2console(kLevelInfo, __VA_ARGS__)
#define zcwarn2(...) zlogger2console(kLevelWarn, __VA_ARGS__)
#define zcerror2(...) zlogger2console(kLevelError, __VA_ARGS__)
#define zcfatal2(...) zlogger2console(kLevelFatal, __VA_ARGS__)

#endif /* zlogger_hpp */
