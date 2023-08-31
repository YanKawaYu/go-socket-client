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
    NetCore::InitHostAndPort("127.0.0.1", "127.0.0.1", 4321, true, []()->std::string{
        return "{\"XDPWS\":\"b6im5otk4jvc7s8asee6hnmc7l\",\"app_version\":\"3.1\",\"channel_type\":\"yingyongbao\",\"device_id\":\"b31f411f21d4a228\",\"device_name\":\"zxy_mac\",\"network\":\"wifi\",\"system_type\":\"android\",\"timestamp\":\"1574775810\",\"token\":\"a3175d5587fd8cf5cdfe5736965831db\"}";
    });
    NetCore::SetOnConnStatusChange([](ConnectStatus status){
        printf("status to: %d\n", status);
    });
    NetCore::MakeSureConnected();
    while (1) {
        int command = 0;
        scanf("%d", &command);
        if (command == 1) {
            NetCore::Send("chat.GetMessageList", "{}", [](ErrorCode errCode, std::string payload){
                printf("%s\n", payload.c_str());
            });
        }
    }
    return 0;
}
