//
//  log_define.h
//  go-socket-client
//
//  Copyright © 2023 Yan. All rights reserved.
//

#ifndef log_define_h
#define log_define_h

#include <sys/time.h>
#include <stdint.h>

typedef enum {
    kAppenderAsync,
    kAppenderSync,
}AppenderMode;

typedef enum {
    kLevelVerbose = 0,
    kLevelDebug, // Detailed information on the flow through the system.
    kLevelInfo, // Interesting runtime events (startup/shutdown), should be conservative and keep to a minimum.
    kLevelWarn, // Other runtime situations that are undesirable or unexpected, but not necessarily "wrong".
    kLevelError, // Other runtime errors or unexpected conditions.
    kLevelFatal, // Severe errors that cause premature termination.
    kLevelNone, // Special level used to disable all log messages.
} LogLevel;

typedef struct LoggerInfo {
    LogLevel level;
    const char *tag;
    const char *fileName;
    const char *funcName;
    int line;

    struct timeval timeval;
    intmax_t pid;
    intmax_t tid;
    intmax_t maintid;
}LoggerInfo;

extern void ConsoleLog(const LoggerInfo* info, const char* log);

#endif /* log_define_h */
