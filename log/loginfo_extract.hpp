//
//  loginfo_extract.hpp
//  go-socket-client
//
//  Copyright © 2023 Yan. All rights reserved.
//

#ifndef loginfo_extract_hpp
#define loginfo_extract_hpp

#include <stdio.h>

const char* ExtractFileName(const char* _path);
void ExtractFunctionName(const char* _func, char* _func_ret, int _len);

#endif /* loginfo_extract_hpp */
