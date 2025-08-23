#pragma once
#include <Mlib/Object.hpp>
#include <Mlib/Os/Os.hpp>
#include <Mlib/Source_Location.hpp>
#include <Mlib/Std_Hash.hpp>
#include <Mlib/Threads/Safe_Atomic_Shared_Mutex.hpp>
#include <atomic>
#include <shared_mutex>
#include <type_traits>
#include <unordered_map>

namespace Mlib {

template <class T>
class DanglingBaseClassPtr;
template <class T>
class DanglingBaseClassRef;

class DanglingBaseClass: public virtual Object {
    template <class T>
    friend class DanglingBaseClassPtr;
    template <class T>
    friend class DanglingBaseClassRef;
public:
    DanglingBaseClass();
    virtual ~DanglingBaseClass() override;
    void print_references() const;
    void assert_no_references() const;
    size_t nreferences() const;
    // template <class T>
    // DanglingBaseClassPtr<T> ptr(SourceLocation loc) const { return DanglingBaseClassPtr<T>(*const_cast<DanglingBaseClass*>(this), loc); }
    // template <class T>
    // DanglingBaseClassRef<T> ref(SourceLocation loc) const { return DanglingBaseClassRef<T>(*const_cast<DanglingBaseClass*>(this), loc); }
private:
    void add_source_location(const void* ptr, SourceLocation loc);
    void remove_source_location(const void* ptr);
    const SourceLocation& loc(const void* ptr) const;

    std::unordered_map<const void*, SourceLocation> locs_;
    mutable SafeAtomicSharedMutex loc_mutex_;
};

struct CopyDanglingClassPtr {};
struct MoveDanglingClassPtr {};

template <class T>
class DanglingBaseClassPtr {
    template <class T2>
    friend class DanglingBaseClassPtr;
    template <class T2>
    friend class DanglingBaseClassRef;
public:
    DanglingBaseClassPtr(std::nullptr_t)
        : b_{ nullptr }
        , v_{ nullptr }
    {}
    DanglingBaseClassPtr(DanglingBaseClassPtr&& other)
        : DanglingBaseClassPtr{ std::move(other), MoveDanglingClassPtr() }
    {}
    DanglingBaseClassPtr(const DanglingBaseClassPtr& other)
        : DanglingBaseClassPtr{ other, CopyDanglingClassPtr() }
    {}
    DanglingBaseClassPtr& operator = (const DanglingBaseClassPtr& other) {
        return assign(other);
    }
    template <typename TDerived>
        requires std::is_convertible_v<TDerived&, T&>
    DanglingBaseClassPtr(DanglingBaseClassPtr<TDerived>&& other)
        : DanglingBaseClassPtr{ std::move(other), MoveDanglingClassPtr() }
    {}
    template <typename TDerived>
        requires std::is_convertible_v<TDerived&, T&>
    DanglingBaseClassPtr(const DanglingBaseClassPtr<TDerived>& other)
        : DanglingBaseClassPtr{ other, CopyDanglingClassPtr() }
    {}
    template <typename TDerived>
        requires std::is_convertible_v<TDerived&, T&>
    DanglingBaseClassPtr& operator = (const DanglingBaseClassPtr<TDerived>& other) {
        return assign(other);
    }
    // explicit DanglingBaseClassPtr(DanglingBaseClass& b, SourceLocation loc)
    //     : b_{&b}
    //     , v_{dynamic_cast<T*>(&b)}
    // {
    //     if (v_ == nullptr) {
    //         verbose_abort("DanglingBaseClassPtr: Cannot cast to desired type");
    //     }
    //     b_->add_source_location(this, loc);
    // }
    template <typename TDerived>
        requires std::is_convertible_v<TDerived&, T&>
    DanglingBaseClassPtr(TDerived& b, SourceLocation loc)
        : DanglingBaseClassPtr{ const_cast<std::remove_const_t<TDerived>&>(b), b, loc }
    {}
    template <typename TDerived>
        requires std::is_convertible_v<TDerived&, T&>
    DanglingBaseClassPtr(TDerived* b, SourceLocation loc)
        : DanglingBaseClassPtr{ const_cast<std::remove_const_t<TDerived>*>(b), b, loc }
    {}
    DanglingBaseClassPtr& operator = (std::nullptr_t) {
        if (v_ != nullptr) {
            b_->remove_source_location(this);
            v_ = nullptr;
        }
        return *this;
    }
    ~DanglingBaseClassPtr() {
        *this = nullptr;
    }
    DanglingBaseClassPtr set_loc(SourceLocation loc) const {
        return DanglingBaseClassPtr{ b_, v_, loc };
    }
    DanglingBaseClassRef<T> operator * () {
        if (v_ == nullptr) {
            verbose_abort("DanglingBaseClassPtr: Nullptr dereference");
        }
        return DanglingBaseClassRef<T>(*b_, *v_, b_->loc(this));
    }
    DanglingBaseClassRef<T> operator * () const {
        if (v_ == nullptr) {
            verbose_abort("DanglingBaseClassPtr: Nullptr dereference");
        }
        return DanglingBaseClassRef<T>(*b_, *v_, b_->loc(this));
    }
    T* operator -> () const {
        return v_;
    }
    T* get() const {
        return v_;
    }
    // Comparison
    bool operator == (std::nullptr_t) const {
        return v_ == nullptr;
    }
    bool operator != (std::nullptr_t) const {
        return v_ != nullptr;
    }
    template <typename TOther>
    requires std::is_convertible_v<TOther&, const T&>
    std::strong_ordering operator <=> (const DanglingBaseClassPtr<TOther>& other) const {
        return v_ <=> other.v_;
    }
    template <typename TOther>
        requires std::is_convertible_v<TOther&, const T&>
    bool operator == (const DanglingBaseClassPtr<TOther>& other) const {
        return v_ == other.v_;
    }
    template <typename TOther>
        requires std::is_convertible_v<TOther&, const T&>
    bool operator != (const DanglingBaseClassPtr<TOther>& other) const {
        return v_ != other.v_;
    }
    void print_base_references() const {
        b_->print_references();
    }
    SourceLocation loc() const {
        return b_->loc(this);
    }
private:
    DanglingBaseClassPtr(DanglingBaseClass& b, T& v, SourceLocation loc)
        : b_{ &b }
        , v_{ &v }
    {
        b_->add_source_location(this, loc);
    }
    DanglingBaseClassPtr(DanglingBaseClass* b, T* v, SourceLocation loc)
        : b_{ b }
        , v_{ v }
    {
        if (v_ != nullptr) {
            b_->add_source_location(this, loc);
        }
    }
    template <typename TDerived>
        requires std::is_convertible_v<TDerived&, T&>
    DanglingBaseClassPtr(DanglingBaseClassPtr<TDerived>&& other, MoveDanglingClassPtr)
        : b_{ other.b_ }
        , v_{ other.v_ }
    {
        if (v_ != nullptr) {
            b_->add_source_location(this, b_->loc(&other));
            b_->remove_source_location(&other);
            other.v_ = nullptr;
        }
    }
    template <typename TDerived>
        requires std::is_convertible_v<TDerived&, T&>
    DanglingBaseClassPtr(const DanglingBaseClassPtr<TDerived>& other, CopyDanglingClassPtr)
        : b_{ other.b_ }
        , v_{ other.v_ }
    {
        if (v_ != nullptr) {
            b_->add_source_location(this, b_->loc(&other));
        }
    }
    template <typename TDerived>
        requires std::is_convertible_v<TDerived&, T&>
    DanglingBaseClassPtr& assign(const DanglingBaseClassPtr<TDerived>& other) {
        if (this == (void*)&other) {
            return *this;
        }
        if (v_ != nullptr) {
            b_->remove_source_location(this);
        }
        b_ = other.b_;
        v_ = other.v_;
        if (v_ != nullptr) {
            b_->add_source_location(this, b_->loc(&other));
        }
        return *this;
    }
    DanglingBaseClass* b_;
    T* v_;
};

template <class T>
class DanglingBaseClassRef {
    template <class T2>
    friend class DanglingBaseClassPtr;
    template <class T2>
    friend class DanglingBaseClassRef;
    DanglingBaseClassRef& operator = (const DanglingBaseClassRef&) = delete;
public:
    DanglingBaseClassRef(DanglingBaseClassRef&& other)
        : DanglingBaseClassRef{ other.b_, other.v_, other.b_.loc(&other) }
    {}
    DanglingBaseClassRef(const DanglingBaseClassRef& other)
        : DanglingBaseClassRef{ other.b_, other.v_, other.b_.loc(&other) }
    {}
    template <typename TDerived>
        requires std::is_convertible_v<TDerived&, T&>
    DanglingBaseClassRef(DanglingBaseClassRef<TDerived>&& other)
        : DanglingBaseClassRef{ other.b_, other.v_, other.b_.loc(&other) }
    {}
    template <typename TDerived>
        requires std::is_convertible_v<TDerived&, T&>
    DanglingBaseClassRef(const DanglingBaseClassRef<TDerived>& other)
        : DanglingBaseClassRef{ other.b_, other.v_, other.b_.loc(&other) }
    {}
    template <class TDerived>
        requires std::is_convertible_v<TDerived&, T&>
    DanglingBaseClassRef(TDerived& b, SourceLocation loc)
        : DanglingBaseClassRef{ const_cast<TDerived&>(b), b, loc }
    {}
    ~DanglingBaseClassRef() {
        b_.remove_source_location(this);
    }
    DanglingBaseClassRef set_loc(SourceLocation loc) const {
        return DanglingBaseClassRef{ b_, v_, loc };
    }
    DanglingBaseClassPtr<T> ptr() const {
        return DanglingBaseClassPtr<T>{ b_, v_, b_.loc(this) };
    }
    T* operator -> () const {
        return &v_;
    }
    T& get() const {
        return v_;
    }
    void print_base_references() const {
        b_.print_references();
    }
    SourceLocation loc() const {
        return b_.loc(this);
    }
private:
    explicit DanglingBaseClassRef(DanglingBaseClass& b, T& v, SourceLocation loc)
        : b_{ b }
        , v_{ v }
    {
        b_.add_source_location(this, loc);
    }
    DanglingBaseClass& b_;
    T& v_;
};

}

namespace std {

template <class T>
struct hash<Mlib::DanglingBaseClassPtr<T>>
{
    std::size_t operator()(const Mlib::DanglingBaseClassPtr<T>& s) const noexcept {
        return std::hash<T*>()(s.get());
    }
};

}
