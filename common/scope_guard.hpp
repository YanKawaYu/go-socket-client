//
//  ScopeGuard.hpp
//  go-socket-client
//
//  Copyright © 2023 Yan. All rights reserved.
//

#ifndef ScopeGuard_hpp
#define ScopeGuard_hpp

#include <stdio.h>
#include <functional>

class ScopeGuard {
public:
    explicit ScopeGuard(std::function<void()> onExit);
    ~ScopeGuard();

    ScopeGuard(ScopeGuard const&) = delete;
    ScopeGuard& operator=(ScopeGuard const&) = delete;
private:
    std::function<void()> _onExit;
};

#endif /* ScopeGuard_hpp */
