#pragma once
#include <compare>
#include <memory>

namespace Mlib {
template<typename T1, typename T2>
concept pointers_are_comparable = requires(const T2* v) {
    { (T1*)nullptr == v };
};
}

#ifndef WITHOUT_DANGLING_UNIQUE_PTR
#include <Mlib/Os/Os.hpp>
#include <Mlib/Source_Location.hpp>
#include <Mlib/Threads/Safe_Atomic_Shared_Mutex.hpp>
#include <cstdint>
#include <mutex>
#include <shared_mutex>
#include <type_traits>
#include <unordered_map>

namespace Mlib {

struct PointedSourceLocation;

template <class T>
std::unordered_map<const void*, PointedSourceLocation>& locs_();
template <class T>
SafeAtomicSharedMutex& loc_mutex_();

template <class T>
inline std::unordered_map<const void*, ::Mlib::PointedSourceLocation>& locs() {
    return locs_<std::remove_const_t<T>>();
}

template <class T>
inline SafeAtomicSharedMutex& loc_mutex() {
    return loc_mutex_<std::remove_const_t<T>>();
}

#define DP_IMPLEMENT(T)                                                                 \
    template <>                                                                         \
    std::unordered_map<const void*, ::Mlib::PointedSourceLocation>& Mlib::locs_<T>() {  \
        static std::unordered_map<const void*, ::Mlib::PointedSourceLocation> result;   \
        return result;                                                                  \
    }                                                                                   \
    template <>                                                                         \
    ::Mlib::SafeAtomicSharedMutex& ::Mlib::loc_mutex_<T>() {                            \
        static SafeAtomicSharedMutex result;                                            \
        return result;                                                                  \
    }

#define SOURCE_LOCATION SourceLocation
#define DP_LOC CURRENT_SOURCE_LOCATION

using ReferenceCounter = std::uint32_t;
using MagicNumber = uint32_t;
static_assert(sizeof(ReferenceCounter) == 4);

template <class T>
void print_source_locations(const ReferenceCounter& v) {
    for (const auto& [ptr, psl] : locs<T>()) {
        if (psl.target == &v) {
            lerr() << psl.loc.file_name() << ':' << psl.loc.line();
        }
    }
}

template <class T>
struct ObjectAndReferenceCounter {
    template<class... Args>
    ObjectAndReferenceCounter(Args&&... args)
        : nptrs{ 0 }
    {
        new(obj) T(std::forward<Args>(args)...);
    }
    T& tobj() {
        return reinterpret_cast<T&>(obj);
    }
    void destroy() {
        {
            std::shared_lock lock{ loc_mutex<T>() };
            if (nptrs != 0) {
                print_source_locations<T>(erase_type(*this));
                verbose_abort("DanglingUniquePtr: " + std::to_string(nptrs) + " dangling pointers remain");
            }
        }
        tobj().~T();
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
    SourceLocation loc;
};

template <class T>
void add_source_location(const void* ptr, const ReferenceCounter& v, SourceLocation loc) {
    auto res = locs<T>().insert({ptr, {&v, loc}});
    if (!res.second) {
        print_source_locations<T>(*res.first->second.target);
        verbose_abort("Could not insert source location");
    }
}

template <class T>
void remove_source_location(const void* ptr) {
    if (locs<T>().erase(ptr) != 1) {
        verbose_abort("Could not erase source location");
    }
}

template <class T>
void check_consistency(const ReferenceCounter& v) {
    uint32_t count = 0;
    for (const auto& [ptr, psl] : locs<T>()) {
        if (psl.target == &v) {
            ++count;
        }
    }
    if (count != v) {
        print_source_locations<T>(v);
        verbose_abort("Inconsistent count. Locations: " + std::to_string(count) + ". Counter: " + std::to_string(v));
    }
}

template <class T>
class DanglingPtr;

template <class T>
class DanglingRef;

template <class T>
class DanglingUniquePtr {
    DanglingUniquePtr() = delete;
    DanglingUniquePtr(const DanglingUniquePtr&) = delete;
    DanglingUniquePtr& operator = (const DanglingUniquePtr&) = delete;
public:
    DanglingUniquePtr(std::nullopt_t)
        : u_{ nullptr }
    {}
    explicit DanglingUniquePtr(std::unique_ptr<ObjectAndReferenceCounter<T>>&& u)
        : u_{ std::move(u) }
    {}
    DanglingUniquePtr(DanglingUniquePtr&& u) noexcept
        : u_{ std::move(u.u_) }
    {}
    DanglingUniquePtr& operator = (DanglingUniquePtr&& u) noexcept {
        u_ = std::move(u.u_);
        return *this;
    }
    DanglingUniquePtr& operator = (std::nullptr_t) {
        deallocate();
        return *this;
    }
    ~DanglingUniquePtr() {
        deallocate();
    }
    DanglingPtr<T> get(SourceLocation loc) const {
        if (u_ == nullptr) {
            return DanglingPtr<T>{nullptr};
        } else {
            return DanglingPtr<T>{erase_type(*u_), loc};
        }
    }
    DanglingRef<T> ref(SourceLocation loc) const {
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
    ReferenceCounter nreferences() const {
        if (u_ == nullptr) {
            verbose_abort("Attempt to retrieve nreferences of nullptr");
        }
        std::scoped_lock lock{ loc_mutex<T>() };
        return u_->nptrs;
    }
    void print_references() const {
        if (u_ == nullptr) {
            verbose_abort("Attempt to print print_references of nullptr");
        }
        std::scoped_lock lock{ loc_mutex<T>() };
        print_source_locations<T>(u_->nptrs);
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
    void deallocate() {
        if (u_ == nullptr) {
            return;
        }
        u_->destroy();
        u_ = nullptr;
    }
    std::unique_ptr<ObjectAndReferenceCounter<T>> u_;
};

template <class T>
class DanglingPtr {
    friend DanglingPtr<const T>;
    friend DanglingPtr<std::remove_const_t<T>>;
public:
    static DanglingPtr from_object(T& v, SourceLocation loc)
    {
        return DanglingPtr{counter_from_object(v), loc};
    }
    // Constructor from pointer
    DanglingPtr(std::nullptr_t)
        : u_{ nullptr }
        , loc_{ CURRENT_SOURCE_LOCATION }
    {}
    // Constructor from ReferenceCounter
    DanglingPtr(ReferenceCounter& u, SourceLocation loc) : u_{ &u }, loc_{ loc } {
        std::scoped_lock lock{ loc_mutex<T>() };
        add_source_location<T>(this, *u_, loc);
        inc(*u_);
        // check_consistency<T>(*u_);
    }
    // Copy-constructor from DanglingPtr
    DanglingPtr(const DanglingPtr& other) : u_{ other.u_ }, loc_{ other.loc_ } {
        if (u_ != nullptr) {
            std::scoped_lock lock{ loc_mutex<T>() };
            add_source_location<T>(this, *u_, loc_);
            inc(*u_);
            // check_consistency<T>(*u_);
        }
    }
    DanglingPtr(DanglingPtr&& other) : u_{ other.u_ }, loc_{ other.loc_ } {
        if (u_ != nullptr) {
            std::scoped_lock lock{ loc_mutex<T>() };
            remove_source_location<T>(&other);
            add_source_location<T>(this, *u_, loc_);
            other.u_ = nullptr;
            // check_consistency<T>(*u_);
        }
    }
    DanglingPtr set_loc(SourceLocation loc) const {
        if (u_ == nullptr) {
            return nullptr;
        } else {
            return DanglingPtr{ *u_, loc };
        }
    }
    T& release() {
        if (u_ == nullptr) {
            verbose_abort("Attempt to release nullptr");
        }
        std::scoped_lock lock{ loc_mutex<T>() };
        remove_source_location<T>(this);
        dec(*u_);
        // check_consistency<T>(*u_);
        T& res = data<T>(*u_);
        u_ = nullptr;
        return res;
    }
    // Assignment operator from DanglingPtr
    DanglingPtr& operator = (const DanglingPtr& other) {
        std::scoped_lock lock{ loc_mutex<T>() };
        if (u_ != nullptr) {
            remove_source_location<T>(this);
            dec(*u_);
            // check_consistency<T>(*u_);
        }
        u_ = other.u_;
        loc_ = other.loc_;
        if (u_ != nullptr) {
            add_source_location<T>(this, *u_, loc_);
            inc(*u_);
            // check_consistency<T>(*u_);
        }
        return *this;
    }
    DanglingPtr& operator = (DanglingPtr&& other) {
        if (u_ == other.u_) {
            return *this;
        }
        std::scoped_lock lock{ loc_mutex<T>() };
        if (u_ != nullptr) {
            remove_source_location<T>(this);
            dec(*u_);
            // check_consistency<T>(*u_);
        }
        u_ = other.u_;
        loc_ = other.loc_;
        if (u_ != nullptr) {
            remove_source_location<T>(&other);
            add_source_location<T>(this, *u_, loc_);
            other.u_ = nullptr;
            // check_consistency<T>(*u_);
        }
        return *this;
    }
    // Misc
    ~DanglingPtr() {
        if (u_ != nullptr) {
            std::scoped_lock lock{ loc_mutex<T>() };
            remove_source_location<T>(this);
            dec(*u_);
            // check_consistency<T>(*u_);
        }
    }
    operator const DanglingPtr<const T>&() const {
        return *reinterpret_cast<const DanglingPtr<const T>*>(this);
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
    T* get() const {
        if (u_ == nullptr) {
            return nullptr;
        }
        return &data<T>(*u_);
    }
    T& obj() const {
        return data<T>(*u_);
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
    SourceLocation loc_;
};

template <class T>
class DanglingRef {
    DanglingRef& operator = (const DanglingRef&) = delete;
public:
    static DanglingRef from_object(T& v, SourceLocation loc) {
        return DanglingRef{counter_from_object(v), loc};
    }
    // Constructor from ReferenceCounter
    DanglingRef(ReferenceCounter& u, SourceLocation loc)
        : u_{u}
        , loc_{loc}
    {
        std::scoped_lock lock{ loc_mutex<T>() };
        add_source_location<T>(this, u_, loc);
        inc(u_);
        // check_consistency<T>(u_);
    }
    DanglingRef(const DanglingRef& other)
        : DanglingRef{other.u_, other.loc_}
    {}
    DanglingRef(DanglingRef&& other) noexcept
        : DanglingRef{other.u_, other.loc_}
    {}
    ~DanglingRef() {
        std::scoped_lock lock{ loc_mutex<T>() };
        remove_source_location<T>(this);
        dec(u_);
        // check_consistency<T>(u_);
    }
    DanglingRef set_loc(SourceLocation loc) const {
        return DanglingRef{ u_, loc };
    }
    // T& release() {
    //     dec(u_);
    //     T& res = data<T>(u_);
    //     return res;
    // }
    operator const DanglingRef<const T>&() const {
        return *reinterpret_cast<const DanglingRef<const T>*>(this);
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
    T& obj() const {
        return data<T>(u_);
    }
private:
    ReferenceCounter& u_;
    SourceLocation loc_;
};

template< class T, class... Args >
DanglingUniquePtr<T> make_dunique( Args&&... args ) {
    return DanglingUniquePtr{std::make_unique<ObjectAndReferenceCounter<T>>(std::forward<Args>(args)...)};
}

}

#else

#define DP_IMPLEMENT(T)
using SOURCE_LOCATION = int;
static const SOURCE_LOCATION DP_LOC = 42;

namespace Mlib {

template <class T>
class DanglingPtr;

template <class T>
class DanglingRef;

template <class T>
class DanglingUniquePtr {
public:
    explicit DanglingUniquePtr(std::unique_ptr<T>&& u)
    : u_{std::move(u)}
    {}
    DanglingUniquePtr(DanglingUniquePtr&& u)
    : u_{std::move(u.u_)}
    {}
    DanglingUniquePtr& operator = (DanglingUniquePtr&& u) {
        u_ = std::move(u.u_);
        return *this;
    }
    DanglingUniquePtr& operator = (std::nullptr_t) {
        u_ = nullptr;
        return *this;
    }
    ~DanglingUniquePtr() = default;
    DanglingPtr<T> get(SOURCE_LOCATION) const {
        return DanglingPtr<T>{u_.get()};
    }
    DanglingRef<T> ref(SOURCE_LOCATION) const {
        return DanglingRef<T>{*u_};
    }
    T* operator -> () {
        return u_.get();
    }
    const T* operator -> () const {
        return u_.get();
    }
    // Comparison
    bool operator == (std::nullptr_t) const {
        return u_ == nullptr;
    }
    bool operator != (std::nullptr_t) const {
        return u_ != nullptr;
    }
    std::strong_ordering operator <=> (const DanglingUniquePtr& other) const {
        return u_ <=> other;
    }
private:
    std::unique_ptr<T> u_;
};

template <class T>
class DanglingStackPtr {
public:
    DanglingStackPtr() = default;
    ~DanglingStackPtr() = default;
    DanglingPtr<T> get(SOURCE_LOCATION) const {
        return DanglingPtr<T>{const_cast<T*>(&u_)};
    }
    DanglingRef<T> ref(SOURCE_LOCATION) const {
        return DanglingRef<T>{const_cast<T&>(u_)};
    }
    T* operator -> () {
        return &u_;
    }
    const T* operator -> () const {
        return &u_;
    }
private:
    T u_;
};

template <class T>
class DanglingPtr {
    friend DanglingPtr<const T>;
    friend DanglingPtr<std::remove_const_t<T>>;
public:
    static DanglingPtr from_object(T& v, SOURCE_LOCATION)
    {
        return DanglingPtr{&v};
    }
    DanglingPtr(std::nullptr_t): u_{nullptr} {}
    explicit DanglingPtr(T* u): u_{u} {}
    DanglingPtr(const DanglingPtr& other) : u_{other.u_} {}
    DanglingPtr(DanglingPtr&& other) noexcept : u_{other.u_} {}
    DanglingPtr set_loc(SOURCE_LOCATION) const { return *this; }
    T& release() {
        T& res = *u_;
        u_ = nullptr;
        return res;
    }
    DanglingPtr& operator = (std::nullptr_t) {
        u_ = nullptr;
        return *this;
    }
    DanglingPtr& operator = (const DanglingPtr& other) {
        u_ = other.u_;
        return *this;
    }
    DanglingPtr& operator = (DanglingPtr&& other) {
        u_ = other.u_;
        return *this;
    }
    ~DanglingPtr() = default;
    operator DanglingPtr<const T>() const {
        return DanglingPtr<const T>{u_};
    }
    DanglingRef<T> operator * () const {
        return DanglingRef<T>{*u_};
    }
    T* operator -> () const {
        return u_;
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
    T* u_;
};

template <class T>
class DanglingRef {
public:
    static DanglingRef from_object(T& v, SOURCE_LOCATION) {
        return DanglingRef{v};
    }
    explicit DanglingRef(T& u): u_{u} {}
    DanglingRef(const DanglingRef& other): DanglingRef{other.u_}
    {}
    ~DanglingRef() = default;
    DanglingRef set_loc(SOURCE_LOCATION) const { return *this; }
    operator DanglingRef<const T>() const {
        return DanglingRef<const T>{u_};
    }
    DanglingPtr<T> ptr() const {
        return DanglingPtr<T>{&u_};
    }
    T* operator -> () const {
        return &u_;
    }
    bool operator < (const T& p) const {
        return u_ < p;
    }
    bool operator == (const T& p) const {
        return u_ == p;
    }
    bool operator != (const T& p) const {
        return u_ != p;
    }
private:
    T& u_;
};

template< class T, class... Args >
DanglingUniquePtr<T> make_dunique( Args&&... args ) {
    return DanglingUniquePtr{std::make_unique<T>(std::forward<Args>(args)...)};
}

}

#endif
