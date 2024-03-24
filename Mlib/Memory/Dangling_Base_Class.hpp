#pragma once
#include <Mlib/Object.hpp>
#include <Mlib/Os/Os.hpp>
#include <Mlib/Source_Location.hpp>
#include <atomic>
#include <shared_mutex>
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
    template <class T>
    DanglingBaseClassPtr<T> ptr(SourceLocation loc) const { return DanglingBaseClassPtr<T>(*const_cast<DanglingBaseClass*>(this), loc); }
    template <class T>
    DanglingBaseClassRef<T> ref(SourceLocation loc) const { return DanglingBaseClassRef<T>(*const_cast<DanglingBaseClass*>(this), loc); }
private:
    void add_source_location(const void* ptr, SourceLocation loc);
    void remove_source_location(const void* ptr);
    const SourceLocation& loc(const void* ptr) const;

    std::unordered_map<const void*, SourceLocation> locs_;
    mutable std::shared_mutex loc_mutex_;
};

template <class T>
class DanglingBaseClassPtr {
    DanglingBaseClassPtr() = delete;
public:
    DanglingBaseClassPtr(std::nullptr_t)
        : b_{nullptr}
        , v_{nullptr}
    {}
    DanglingBaseClassPtr(DanglingBaseClassPtr<T>&& other)
        : b_{other.b_}
        , v_{other.v_}
    {
        if (v_ != nullptr) {
            b_->add_source_location(this, b_->loc(&other));
            b_->remove_source_location(&other);
            other.v_ = nullptr;
        }
    }
    DanglingBaseClassPtr(const DanglingBaseClassPtr<T>& other)
        : b_{other.b_}
        , v_{other.v_}
    {
        if (v_ != nullptr) {
            b_->add_source_location(this, b_->loc(&other));
        }
    }
    DanglingBaseClassPtr& operator = (const DanglingBaseClassPtr<T>& other) {
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
    DanglingBaseClassPtr(DanglingBaseClass& b, SourceLocation loc)
        : b_{&b}
        , v_{dynamic_cast<T*>(&b)}
    {
        if (v_ == nullptr) {
            verbose_abort("DanglingBaseClassPtr: Cannot cast to desired type");
        }
        b_->add_source_location(this, loc);
    }
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
    DanglingBaseClassRef<T> operator * () {
        if (v_ == nullptr) {
            verbose_abort("DanglingBaseClassPtr: Nullptr dereference");
        }
        return DanglingBaseClassRef<T>(*b_, b_->loc(this));
    }
    DanglingBaseClassRef<T> operator * () const {
        if (v_ == nullptr) {
            verbose_abort("DanglingBaseClassPtr: Nullptr dereference");
        }
        return DanglingBaseClassRef<T>(*b_, b_->loc(this));
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
    std::strong_ordering operator <=> (const DanglingBaseClassPtr& other) const {
        return v_ <=> other.v_;
    }
private:
    DanglingBaseClass* b_;
    T* v_;
};

template <class T>
class DanglingBaseClassRef {
    DanglingBaseClassRef(DanglingBaseClassRef&&) = delete;
    DanglingBaseClassRef(const DanglingBaseClassRef&) = delete;
    DanglingBaseClassRef& operator = (const DanglingBaseClassRef&) = delete;
public:
    DanglingBaseClassRef(DanglingBaseClass& b, SourceLocation loc)
        : b_{b}
        , v_{dynamic_cast<T*>(&b)}
    {
        if (v_ == nullptr) {
            verbose_abort("DanglingBaseClassRef: Cannot cast to desired type");
        }
        b_.add_source_location(this, loc);
    }
    ~DanglingBaseClassRef() {
        b_.remove_source_location(this);
    }
    DanglingBaseClassPtr<T> ptr() const {
        return DanglingBaseClassPtr<T>(b_, b_.loc(this));
    }
    T* operator -> () const {
        return v_;
    }
private:
    DanglingBaseClass& b_;
    T* v_;
};

}
