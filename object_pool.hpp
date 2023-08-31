//
//  object_pool.hpp
//  go-socket-client
//
//  Copyright © 2023 Yan. All rights reserved.
//

#ifndef object_pool_hpp
#define object_pool_hpp

#include <stdio.h>
#include <queue>
#include <mutex>

template <class T>
class ObjectPool {
public:
    using Destructor = std::function<void(T*)>;

    std::unique_ptr<T, typename ObjectPool<T>::Destructor> Get() {
        std::lock_guard<std::mutex> lock(_mutex);
        T *t;
        if (_pool.empty()) {
            t = new T();
        }else {
            t = _pool.back().release();
            _pool.pop_back();
        }
        std::unique_ptr<T, Destructor> ptr(t, [this](T *t) {
            _pool.push_back(std::unique_ptr<T>(t));
        });
        return ptr;
    };
private:
    std::vector<std::unique_ptr<T>> _pool;
    std::mutex _mutex;
};

#endif /* object_pool_hpp */
