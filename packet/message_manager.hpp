//
//  message_manager.hpp
//  go-socket-client
//
//  Copyright © 2023 Yan. All rights reserved.
//

#ifndef message_manager_hpp
#define message_manager_hpp

#include <stdio.h>
#include "message.hpp"
#include <memory>

class MessageManager {
public:
    std::unique_ptr<MessageBase> DecodeMessage(IReader *reader);
    void EncodeMessage(IWriter *writer, MessageBase *message);
};

#endif /* message_manager_hpp */
