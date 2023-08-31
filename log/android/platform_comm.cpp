//
//  platform_comm.cpp
//  go-socket-client
//
//  Copyright © 2023 Yan. All rights reserved.
//

#include <stdio.h>
#include "../log_define.h"
#include "../loginfo_extract.hpp"
#include <android/log.h>

void ConsoleLog(const LoggerInfo* info, const char* log) {
    char result_log[2048] = {0};
    if (info) {
        const char* filename = ExtractFileName(info->fileName);
        char strFuncName [128] = {0};
        ExtractFunctionName(info->funcName, strFuncName, sizeof(strFuncName));

        snprintf(result_log,  sizeof(result_log), "[%s, %s, %d]:%s", filename, strFuncName, info->line, log?log:"NULL==log!!!");
        __android_log_print(info->level+2, info->tag?info->tag:"", "%s", (const char*)result_log);
    } else {
        snprintf(result_log,  sizeof(result_log) , "%s", log?log:"NULL==log!!!");
        __android_log_print(ANDROID_LOG_WARN, "", "%s", (const char*)result_log);
    }
}
