#pragma once
#include <Mlib/Os/Os.hpp>
#include <atomic>
#include <compare>
#include <cstdint>
#include <map>
#include <memory>
#include <mutex>
#include <source_location>
#include <type_traits>

namespace Mlib {

#define DP_LOC std::source_location::current()

using ReferenceCounter = std::atomic_uint32_t;
using MagicNumber = uint32_t;
static_assert(sizeof(ReferenceCounter) == 4);

template <class T>
struct ObjectAndReferenceCounter {
    template<class... Args>
    ObjectAndReferenceCounter(Args&&... args)
    : nptrs{0}
    {
        new(obj) T(std::forward<Args>(args)...);
    }
    T& tobj() {
        return reinterpret_cast<T&>(obj);
    }
    ReferenceCounter nptrs;
    MagicNumber magic_number = 0xc0ffee42u;
    char obj[sizeof(T)];
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
ReferenceCounter& counter_from_object(const T& v) {
    static_assert(sizeof(ObjectAndReferenceCounter<T>) == sizeof(ReferenceCounter) + sizeof(MagicNumber) + sizeof(T));
    const auto& obj = *reinterpret_cast<const ObjectAndReferenceCounter<T>*>(reinterpret_cast<const ReferenceCounter*>(&v) - 2);
    if (obj.magic_number != 0xc0ffee42u) {
        verbose_abort("Incorrect magic number during reference counting");
    }
    return const_cast<ReferenceCounter&>(obj.nptrs);
}

struct PointedSourceLocation {
    const ReferenceCounter* target;
    std::source_location loc;
};

template <class T>
std::map<const void*, PointedSourceLocation>& locs() {
    static std::map<const void*, PointedSourceLocation> result;
    return result;
}

template <class T>
std::mutex& loc_mutex() {
    static std::mutex result;
    return result;
}

template <class T>
void add_source_location(const void* ptr, const ReferenceCounter& v, std::source_location loc) {
    std::scoped_lock lock{loc_mutex<T>()};
    if (!locs<T>().insert({ptr, {&v, loc}}).second) {
        verbose_abort("Could not insert source location");
    }
}

template <class T>
void remove_source_location(const void* ptr) {
    std::scoped_lock lock{loc_mutex<T>()};
    if (locs<T>().erase(ptr) != 1) {
        verbose_abort("Could not erase source location");
    }
}

template <class T>
void print_source_locations(const ReferenceCounter& v) {
    using NT = std::remove_const_t<T>;
    using CT = const T;
    {
        lerr() << "Non-const:";
        std::scoped_lock lock{loc_mutex<NT>()};
        for (const auto& [ptr, psl] : locs<NT>()) {
            if (psl.target == &v) {
                lerr() << psl.loc.file_name() << ':' << psl.loc.line();
            }
        }
    }
    {
        lerr() << "Const:";
        std::scoped_lock lock{loc_mutex<CT>()};
        for (const auto& [ptr, psl] : locs<CT>()) {
            if (psl.target == &v) {
                lerr() << psl.loc.file_name() << ':' << psl.loc.line();
            }
        }
    }
}

template <class T>
void check_consistency(const ReferenceCounter& v) {
    using NT = std::remove_const_t<T>;
    using CT = const T;

    uint32_t count = 0;
    for (const auto& [ptr, psl] : locs<NT>()) {
        if (psl.target == &v) {
            ++count;
        }
    }
    for (const auto& [ptr, psl] : locs<CT>()) {
        if (psl.target == &v) {
            ++count;
        }
    }
    if (count != v) {
        verbose_abort("Inconsistent count. Locations: " + std::to_string(count) + ". Counter: " + std::to_string(v));
    }
}

template<typename T1, typename T2>
concept pointers_are_comparable = requires(const T2* v) {
    { (T1*)nullptr == v };
};

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
        if (u_ == nullptr) {
            return;
        }
        u_->tobj().~T();
        if (u_->nptrs != 0) {
            print_source_locations<T>(erase_type(*u_));
            verbose_abort("DanglingUniquePtr: " + std::to_string(u_->nptrs) + " dangling pointers remain");
        }
    }
    DanglingPtr<T> get(std::source_location loc) const {
        if (u_ == nullptr) {
            return DanglingPtr<T>{nullptr};
        } else {
            return DanglingPtr<T>{erase_type(*u_), loc};
        }
    }
    DanglingRef<T> ref(std::source_location loc) const {
        if (u_ == nullptr) {
            verbose_abort("Nullptr dereferenciation");
        }
        return DanglingRef<T>{erase_type(*u_), loc};
    }
    T* operator -> () {
        return &u_->tobj();
    }
    const T* operator -> () const {
        return &u_->tobj();
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
        u_.tobj().~T();
        if (u_.nptrs != 0) {
            print_source_locations<T>(erase_type(u_));
            verbose_abort("DanglingStackPtr: " + std::to_string(u_.nptrs) + " dangling pointers remain");
        }
    }
    DanglingPtr<T> get(std::source_location loc) const {
        return DanglingPtr<T>{erase_type(u_), loc};
    }
    DanglingRef<T> ref(std::source_location loc) const {
        return DanglingRef<T>{erase_type(u_), loc};
    }
    T* operator -> () {
        return &u_.tobj();
    }
    const T* operator -> () const {
        return &u_.tobj();
    }
private:
    ObjectAndReferenceCounter<T> u_;
};

template <class T>
class DanglingPtr {
    friend DanglingPtr<const T>;
    friend DanglingPtr<std::remove_const_t<T>>;
public:
    static DanglingPtr from_object(T& v, std::source_location loc)
    {
        return DanglingPtr{counter_from_object(v), loc};
    }
    // Constructor from pointer
    DanglingPtr(std::nullptr_t)
    : u_{nullptr},
      loc_{std::source_location::current()}
    {}
    // Constructor from ReferenceCounter
    explicit DanglingPtr(ReferenceCounter& u, std::source_location loc): u_{&u}, loc_{loc} {
        add_source_location<T>(this, *u_, loc);
        inc(*u_);
        // check_consistency<T>(*u_);
    }
    // Copy-constructor from DanglingPtr
    DanglingPtr(const DanglingPtr& other) : u_{other.u_}, loc_{other.loc_} {
        if (u_ != nullptr) {
            add_source_location<T>(this, *u_, loc_);
            inc(*u_);
            // check_consistency<T>(*u_);
        }
    }
    DanglingPtr(DanglingPtr&& other) : u_{other.u_}, loc_{other.loc_} {
        if (u_ != nullptr) {
            remove_source_location<T>(&other);
            add_source_location<T>(this, *u_, loc_);
            other.u_ = nullptr;
            // check_consistency<T>(*u_);
        }
    }
    void set_loc(std::source_location loc) {
        if (u_ == nullptr) {
            verbose_abort("set_loc of nullptr");
        }
        loc_ = loc;
        remove_source_location<T>(this);
        add_source_location<T>(this, *u_, loc_);
    }
    T& release() {
        if (u_ == nullptr) {
            verbose_abort("Attempt to release nullptr");
        }
        remove_source_location<T>(this);
        dec(*u_);
        T& res = data<T>(*u_);
        u_ = nullptr;
        return res;
    }
    // Assignment operator from DanglingPtr
    void operator = (const DanglingPtr& other) {
        if (u_ != nullptr) {
            remove_source_location<T>(this);
            dec(*u_);
        }
        u_ = other.u_;
        loc_ = other.loc_;
        if (u_ != nullptr) {
            add_source_location<T>(this, *u_, loc_);
            inc(*u_);
            // check_consistency<T>(*u_);
        }
    }
    void operator = (DanglingPtr&& other) {
        if (u_ == other.u_) {
            return;
        }
        if (u_ != nullptr) {
            remove_source_location<T>(this);
            dec(*u_);
        }
        u_ = other.u_;
        loc_ = other.loc_;
        if (u_ != nullptr) {
            remove_source_location<T>(&other);
            add_source_location<T>(this, *u_, loc_);
            other.u_ = nullptr;
            // check_consistency<T>(*u_);
        }
    }
    // Misc
    ~DanglingPtr() {
        if (u_ != nullptr) {
            remove_source_location<T>(this);
            dec(*u_);
            // check_consistency<T>(*u_);
        }
    }
    operator DanglingPtr<const T>() const {
        if (u_ == nullptr) {
            return DanglingPtr<const T>{nullptr};
        } else {
            return DanglingPtr<const T>{*const_cast<ReferenceCounter*>(u_), loc_};
        }
    }
    DanglingRef<T> operator * () const {
        if (u_ == nullptr) {
            verbose_abort("Nullptr dereferenciation");
        }
        return DanglingRef<T>{*u_, loc_};
    }
    T* operator -> () const {
        return &data<T>(*u_);
    }
    template <typename T2>
    requires pointers_are_comparable<T, T2>
    bool operator == (const DanglingPtr<T2>& other) const {
        return u_ == other.u_;
    }
    template <typename T2>
    requires pointers_are_comparable<T, T2>
    bool operator != (const DanglingPtr<T2>& other) const {
        return u_ != other.u_;
    }
    bool operator == (std::nullptr_t) const {
        return u_ == nullptr;
    }
    bool operator != (std::nullptr_t) const {
        return u_ != nullptr;
    }
    std::strong_ordering operator <=> (const DanglingPtr& other) const {
        return u_ <=> other.u_;
    }
private:
    ReferenceCounter* u_;
    std::source_location loc_;
};

template <class T>
class DanglingRef {
public:
    static DanglingRef from_object(T& v, std::source_location loc) {
        return DanglingRef{counter_from_object(v), loc};
    }
    // Constructor from ReferenceCounter
    DanglingRef(ReferenceCounter& u, std::source_location loc): u_{u}, loc_{loc} {
        add_source_location<T>(this, u_, loc);
        inc(u_);
        // check_consistency<T>(u_);
    }
    DanglingRef(const DanglingRef& other): DanglingRef{other.u_, other.loc_}
    {}
    ~DanglingRef() {
        remove_source_location<T>(this);
        dec(u_);
        // check_consistency<T>(u_);
    }
    void set_loc(std::source_location loc) {
        loc_ = loc;
        remove_source_location<T>(this);
        add_source_location<T>(this, u_, loc_);
        // check_consistency<T>(u_);
    }
    // T& release() {
    //     dec(u_);
    //     T& res = data<T>(u_);
    //     return res;
    // }
    operator DanglingRef<const T>() const {
        return DanglingRef<const T>{u_, loc_};
    }
    DanglingPtr<T> ptr() const {
        return DanglingPtr<T>{u_, loc_};
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
    std::source_location loc_;
};

template< class T, class... Args >
DanglingUniquePtr<T> make_dunique( Args&&... args ) {
    return DanglingUniquePtr{std::make_unique<ObjectAndReferenceCounter<T>>(std::forward<Args>(args)...)};
}

}