#pragma once
#include <shared_mutex>

namespace Mlib {

template <class TSingleton>
class SingletonGuard {
    SingletonGuard(const SingletonGuard &) = delete;
    SingletonGuard &operator=(const SingletonGuard &) = delete;

public:
    explicit SingletonGuard(TSingleton &instance);
    ~SingletonGuard();
};

template <class TSingleton>
class Singleton {
    friend SingletonGuard<TSingleton>;
    Singleton() = delete;
    Singleton &operator=(const Singleton &) = delete;

public:
    static TSingleton& instance();

private:
    static TSingleton* instance_;
    static std::shared_mutex mutex_;
};

}
