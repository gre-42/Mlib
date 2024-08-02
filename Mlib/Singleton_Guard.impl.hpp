#pragma once
#include "Singleton_Guard.hpp"
#include <Mlib/Os/Os.hpp>
#include <Mlib/Throw_Or_Abort.hpp>
#include <mutex>
#include <shared_mutex>

namespace Mlib {

template <class TSingleton>
TSingleton* Singleton<TSingleton>::instance_ = nullptr;

template <class TSingleton>
SafeAtomicSharedMutex Singleton<TSingleton>::mutex_;

template <class TSingleton>
SingletonGuard<TSingleton>::SingletonGuard(TSingleton& instance)
{
    std::scoped_lock lock{ Singleton<TSingleton>::mutex_ };
    if (Singleton<TSingleton>::instance_ != nullptr) {
        THROW_OR_ABORT("Singleton instance already set");
    }
    Singleton<TSingleton>::instance_ = &instance;
}

template <class TSingleton>
SingletonGuard<TSingleton>::~SingletonGuard() {
    if (Singleton<TSingleton>::instance_ == nullptr) {
        verbose_abort("Singleton not set in dtor");
    }
    Singleton<TSingleton>::instance_ = nullptr;
}

template <class TSingleton>
TSingleton& Singleton<TSingleton>::instance() {
    std::shared_lock lock{ Singleton<TSingleton>::mutex_ };
    if (instance_ == nullptr) {
        THROW_OR_ABORT("Singleton not set");
    }
    return *instance_;
}

}
