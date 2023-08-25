#pragma once
#include <Mlib/Os/Os.hpp>
#include <atomic>
#include <memory>

namespace Mlib {

template <class T>
class DanglingPtr;

template <class T>
class DanglingUniquePtr {
    friend DanglingPtr<T>;
public:
    DanglingUniquePtr(std::unique_ptr<T>&& u)
    : u_{std::move(u)}
    {}
    ~DanglingUniquePtr() {
        if (nptrs_ != 0) {
            verbose_abort("Dangling pointers remain");
        }
    }
    DanglingPtr<T> get() {
        return DanglingPtr{*this};
    }
    T& ref() {
        return *u_;
    }
    T* operator -> () {
        return u_.get();
    }
private:
    std::unique_ptr<T> u_;
    std::atomic_uint32_t nptrs_;
};

template <class T>
class DanglingPtr {
public:
    DanglingPtr(T* p)
    : u_{nullptr}
    {
        if (p != nullptr) {
            verbose_abort("Pointer is not nullptr");
        }
    }
    DanglingPtr(const DanglingPtr& other) : u_{other.u_} {
        ++u_->nptrs_;
    }
    DanglingPtr(DanglingUniquePtr<T>& u): u_{&u} {
        ++u_->nptrs_;
    }
    DanglingPtr& operator = (const DanglingPtr& other) {
        if (u_ != nullptr) {
            --u_->nptrs_;
        }
        u_ = other.u_;
    }
    ~DanglingPtr() {
        if (u_ != nullptr) {
            --u_->nptrs_;
        }
    }
    T& ref() {
        if (u_ == nullptr) {
            verbose_abort("Ptr is nullptr");
        }
        return u_->ref();
    }
    T* operator -> () {
        return u_->u_.get();
    }
private:
    DanglingUniquePtr<T>* u_;
};

template< class T, class... Args >
DanglingUniquePtr<T> make_dunique( Args&&... args ) {
    return DanglingUniquePtr{std::make_unique<T>(args...)};
}

}
