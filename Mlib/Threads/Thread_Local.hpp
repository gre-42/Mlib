#pragma once
#include <Mlib/Features.hpp>
#include <Mlib/Throw_Or_Abort.hpp>

#ifndef WITHOUT_THREAD_LOCAL

#define THREAD_LOCAL(T) thread_local T

#else

#define THREAD_LOCAL(T) ::Mlib::ThreadLocal<T>

#include <Mlib/Os/Os.hpp>
#include <pthread.h>
#include <stdexcept>

namespace Mlib {

template <class T>
class ThreadLocal {
    ThreadLocal(const ThreadLocal&) = delete;
    ThreadLocal& operator = (const ThreadLocal&) = delete;
public:
    ThreadLocal(const T& initial_value)
    : initial_value_{initial_value}
    {
        if (pthread_key_create(&thread_specific_key_, data_destructor) != 0) {
            THROW_OR_ABORT("Cannot create thread-specific key");
        }
    }
    ~ThreadLocal() {
        if (pthread_key_delete(thread_specific_key_) != 0) {
            verbose_abort("Cannot delete thread-specific key");
        }
    }
    T& operator = (const T& value) {
        auto& v = get();
        v = value;
        return v;
    }
    auto& operator * () {
        return *get();
    }
    T& operator -> () {
        return get();
    }
    operator T& () {
        return get();
    }
    T& get() {
        auto* res = (T*)pthread_getspecific(thread_specific_key_);
        if (res == nullptr) {
            res = new T{initial_value_};
            if (pthread_setspecific(thread_specific_key_, res) != 0) {
                delete res;
                THROW_OR_ABORT("Cannot set thread-specific value");
            }
        }
        return *res;
    }
private:
    static void data_destructor(void* data) {
        delete (T*)data;
    }
    pthread_key_t thread_specific_key_;
    T initial_value_;
};

}

#endif
