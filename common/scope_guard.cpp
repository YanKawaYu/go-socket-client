//
//  ScopeGuard.cpp
//  go-socket-client
//
//  Copyright © 2023 Yan. All rights reserved.
//

#include "scope_guard.hpp"

ScopeGuard::ScopeGuard(std::function<void()> onExit): _onExit(onExit) {

}

ScopeGuard::~ScopeGuard() {
    _onExit();
}
