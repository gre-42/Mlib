#pragma once
#include <atomic>
#include <iostream>

namespace Mlib {

template <class TData>
class resource_ptr;

template <class TData>
class resource_ptr_target {
    friend resource_ptr<TData>;
public:
    explicit resource_ptr_target(TData* data)
    : data_{data}
    {}
    ~resource_ptr_target() {
        if (ref_count_ != 0) {
            lerr() << "WARNING: resource_ptr_target destructor with nonzero refcount";
        } else {
            delete data_;
        }
    }
    resource_ptr<TData> instantiate() {
        return resource_ptr<TData>{this};
    }
private:
    TData* data_;
    std::atomic_size_t ref_count_ = 0;
};

template <class TData>
class resource_ptr {
public:
    resource_ptr() = delete;
    resource_ptr(const resource_ptr&) = delete;
    resource_ptr& operator = (const resource_ptr&) = delete;
    resource_ptr(resource_ptr_target<TData>* target)
    : target_{target}
    {
        ++target_->ref_count_;
    }
    ~resource_ptr() {
        --target_->ref_count_;
    }
    TData& operator * () {
        return *target_->data_;
    }
    TData* operator -> () {
        return target_->data_;
    }
private:
    resource_ptr_target<TData>* target_;
};

}
