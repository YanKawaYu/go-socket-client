//
//  platform_comm.mm
//  go-socket-client
//
//  Copyright © 2023 Yan. All rights reserved.
//

#import <Foundation/Foundation.h>
#include "../log_define.h"
#include "../loginfo_extract.hpp"

void ConsoleLog(const LoggerInfo* info, const char* log)
{
    if (NULL==info || NULL==log) return;

    static const char* levelStrings[] = {
        "V",
        "D",  // debug
        "I",  // info
        "W",  // warn
        "E",  // error
        "F"  // fatal
    };

    char strFuncName[128]  = {0};
    ExtractFunctionName(info->funcName, strFuncName, sizeof(strFuncName));

    const char* file_name = ExtractFileName(info->fileName);

    char logBuff[16 * 1024] = {0};
    snprintf(logBuff, sizeof(logBuff), "[%s][%s][%s, %s, %d][%s", levelStrings[info->level], NULL == info->tag ? "" : info->tag, file_name, strFuncName, info->line, log);

    NSLog(@"%@", [NSString stringWithUTF8String:logBuff]);
}
