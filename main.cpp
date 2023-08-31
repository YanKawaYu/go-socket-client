//
//  main.cpp
//  go-socket-client
//
//  Copyright © 2023 Yan. All rights reserved.
//

#include <iostream>
#include "net_core.hpp"
#include "packet/tcp_error.hpp"
#include "log/zlogger.hpp"
#include <stdlib.h>
#include <vector>
#include "unit_test.hpp"

int main(int argc, const char * argv[]) {
    // insert code here...
    std::cout << "Hello, World!\n";
    NetCore::InitHostAndPort("127.0.0.1", "127.0.0.1", 8080, false, []()->std::string{
        return "{\"username\":\"haha\", \"password\":\"xxxxx\"}";
    });
    NetCore::SetOnConnStatusChange([](ConnectStatus status){
        printf("status to: %d\n", status);
    });
    NetCore::MakeSureConnected();
    return 0;
}
