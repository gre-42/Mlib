#pragma once
#include <Mlib/Os/Os.hpp>
#include <atomic>
#include <compare>
#include <cstdint>
#include <memory>
#include <type_traits>

namespace Mlib {

using ReferenceCounter = std::atomic_uint32_t;
using MagicNumber = uint32_t;
static_assert(sizeof(ReferenceCounter) == 4);


template <class T>
struct ObjectAndReferenceCounter {
    template<class... Args>
    ObjectAndReferenceCounter(Args&&... args)
    : nptrs{0},
      obj(std::forward<Args>(args)...)
    {}
    ReferenceCounter nptrs;
    MagicNumber magic_number = 0xc0ffee42u;
    T obj;
};

inline void inc(ReferenceCounter& v) {
    ++v;
}

inline void dec(ReferenceCounter& v) {
    --v;
}

template <class T>
T& data(ReferenceCounter& v) {
    static_assert(sizeof(ObjectAndReferenceCounter<T>) == sizeof(ReferenceCounter) + sizeof(MagicNumber) + sizeof(T));
    return *reinterpret_cast<T*>(&v + 2);
}

template <class T>
const T& data(const ReferenceCounter& v) {
    static_assert(sizeof(ObjectAndReferenceCounter<T>) == sizeof(ReferenceCounter) + sizeof(MagicNumber) + sizeof(T));
    return *reinterpret_cast<const T*>(&v + 2);
}

template <class T>
ReferenceCounter& erase_type(const ObjectAndReferenceCounter<T>& v) {
    return const_cast<ReferenceCounter&>(reinterpret_cast<const ReferenceCounter&>(v));
}

template <class T>
ReferenceCounter& counter_from_object(T& v) {
    static_assert(sizeof(ObjectAndReferenceCounter<T>) == sizeof(ReferenceCounter) + sizeof(MagicNumber) + sizeof(T));
    auto& obj = *reinterpret_cast<ObjectAndReferenceCounter<T>*>(reinterpret_cast<ReferenceCounter*>(&v) - 2);
    if (obj.magic_number != 0xc0ffee42u) {
        verbose_abort("Incorrect magic number during reference counting");
    }
    return obj.nptrs;
}

template <class T>
class DanglingPtr;

template <class T>
class DanglingRef;

template <class T>
class DanglingUniquePtr {
public:
    explicit DanglingUniquePtr(std::unique_ptr<ObjectAndReferenceCounter<T>>&& u)
    : u_{std::move(u)}
    {}
    DanglingUniquePtr(DanglingUniquePtr&& u)
    : u_{std::move(u.u_)}
    {}
    void operator = (DanglingUniquePtr&& u) {
        u_ = std::move(u.u_);
    }
    void operator = (std::nullptr_t) {
        u_ = nullptr;
    }
    ~DanglingUniquePtr() {
        if ((u_ != nullptr) && (u_->nptrs != 0)) {
            verbose_abort("DanglingUniquePtr: Dangling pointers remain");
        }
    }
    DanglingPtr<T> get() const {
        if (u_ == nullptr) {
            return DanglingPtr<T>{nullptr};
        } else {
            return DanglingPtr<T>{erase_type(*u_)};
        }
    }
    DanglingRef<T> operator * () const {
        if (u_ == nullptr) {
            verbose_abort("Nullptr dereferenciation");
        }
        return DanglingRef<T>{erase_type(*u_)};
    }
    T* operator -> () {
        return &u_->obj;
    }
    const T* operator -> () const {
        return &u_->obj;
    }
    // Comparison
    bool operator == (std::nullptr_t) const {
        return u_ == nullptr;
    }
    bool operator != (std::nullptr_t) const {
        return u_ != nullptr;
    }
    std::strong_ordering operator <=> (const DanglingUniquePtr& other) const {
        return (void*)u_.get() <=> (void*)other.u_.get();
    }
private:
    std::unique_ptr<ObjectAndReferenceCounter<T>> u_;
};

template <class T>
class DanglingStackPtr {
public:
    DanglingStackPtr() = default;
    ~DanglingStackPtr() {
        if (u_.nptrs != 0) {
            verbose_abort("DanglingStackPtr: Dangling pointers remain");
        }
    }
    DanglingPtr<T> get() const {
        return DanglingPtr<T>{erase_type(u_)};
    }
    DanglingRef<T> operator * () const {
        return DanglingRef<T>{erase_type(u_)};
    }
    T* operator -> () {
        return &u_.obj;
    }
    const T* operator -> () const {
        return &u_.obj;
    }
private:
    ObjectAndReferenceCounter<T> u_;
};

template <class T>
class DanglingPtr {
public:
    static DanglingPtr from_object(T& v) {
        return DanglingPtr{counter_from_object(v)};
    }
    // Constructor from pointer
    DanglingPtr(std::nullptr_t)
    : u_{nullptr}
    {}
    // Constructor from ObjectAndReferenceCounter
    explicit DanglingPtr(ReferenceCounter& u): u_{&u} {
        inc(*u_);
    }
    // Copy-constructor from DanglingPtr
    DanglingPtr(const DanglingPtr& other) : u_{other.u_} {
        if (u_ != nullptr) {
            inc(*u_);
        }
    }
    DanglingPtr(DanglingPtr&& other) : u_{other.u_} {
        other.u_ = nullptr;
    }
    // Assignment operator from DanglingPtr
    void operator = (const DanglingPtr& other) {
        if (u_ != nullptr) {
            dec(*u_);
        }
        u_ = other.u_;
        if (u_ != nullptr) {
            inc(*u_);
        }
    }
    void operator = (DanglingPtr&& other) {
        u_ = other.u_;
        other.u_ = nullptr;
    }
    // Misc
    ~DanglingPtr() {
        if (u_ != nullptr) {
            dec(*u_);
        }
    }
    operator DanglingPtr<const T>() const {
        if (u_ == nullptr) {
            return DanglingPtr<const T>{nullptr};
        } else {
            return DanglingPtr<const T>{*const_cast<ReferenceCounter*>(u_)};
        }
    }
    DanglingRef<T> operator * () const {
        if (u_ == nullptr) {
            verbose_abort("Nullptr dereferenciation");
        }
        return DanglingRef<T>{*u_};
    }
    T* operator -> () const {
        return &data<T>(*u_);
    }
    bool operator == (const DanglingPtr<T>& other) const {
        return u_ == other.u_;
    }
    bool operator != (const DanglingPtr<T>& other) const {
        return u_ != other.u_;
    }
    bool operator == (std::nullptr_t) const {
        return u_ == nullptr;
    }
    bool operator != (std::nullptr_t) const {
        return u_ != nullptr;
    }
    std::strong_ordering operator <=> (const DanglingPtr&) const = default;
private:
    ReferenceCounter* u_;
};

template <class T>
class DanglingRef {
public:
    static DanglingRef from_object(T& v) {
        return DanglingRef{counter_from_object(v)};
    }
    // Constructor from ReferenceCounter
    explicit DanglingRef(ReferenceCounter& u): u_{u} {
        inc(u_);
    }
    DanglingRef(const DanglingRef& other) : u_{other.u_} {
        inc(u_);
    }
    ~DanglingRef() {
        dec(u_);
    }
    operator DanglingRef<const T>() const {
        return DanglingRef<const T>{u_};
    }
    DanglingPtr<T> ptr() const {
        return DanglingPtr<T>{u_};
    }
    T* operator -> () const {
        return &data<T>(u_);
    }
    bool operator < (const T& p) const {
        return data<T>(u_) < p;
    }
    bool operator == (const T& p) const {
        return data<T>(u_) == p;
    }
    bool operator != (const T& p) const {
        return data<T>(u_) != p;
    }
private:
    ReferenceCounter& u_;
};

template< class T, class... Args >
DanglingUniquePtr<T> make_dunique( Args&&... args ) {
    return DanglingUniquePtr{std::make_unique<ObjectAndReferenceCounter<T>>(std::forward<Args>(args)...)};
}

}
