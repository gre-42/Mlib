#pragma once
#include <Mlib/Memory/Dangling_Base_Class.hpp>
#include <Mlib/Memory/Destruction_Functions.hpp>
#include <Mlib/Std_Hash.hpp>
#include <compare>
#include <type_traits>

namespace Mlib {

template <class T>
class DestructionFunctionsTokensPtr {
    DestructionFunctionsTokensPtr(const DestructionFunctionsTokensPtr&) = delete;
    DestructionFunctionsTokensPtr& operator = (const DestructionFunctionsTokensPtr&) = delete;
public:
    using value_type = T;

    template <class TDerived>
        requires std::is_convertible_v<TDerived&, T&>
    inline DestructionFunctionsTokensPtr(
        const DanglingBaseClassPtr<TDerived>& o,
        DestructionFunctions& on_destroy,
        SourceLocation loc)
        : object_{ o }
        , on_object_destroy_{ on_destroy, loc }
    {}
    template <class TDerived>
        requires std::is_convertible_v<TDerived&, T&>
    inline DestructionFunctionsTokensPtr(const DanglingBaseClassPtr<TDerived>& o, SourceLocation loc)
        : DestructionFunctionsTokensPtr{ o, o->on_destroy, loc }
    {}
    inline T* operator -> () const {
        return object_.get();
    }
    inline T* get() const {
        return object_.get();
    }
    inline const DanglingBaseClassPtr<T>& object() const {
        return object_;
    }
    inline void on_destroy(std::function<void()> func, SourceLocation loc) const {
        on_object_destroy_.add(std::move(func), std::move(loc));
    }
private:
    DanglingBaseClassPtr<T> object_;
    mutable DestructionFunctionsRemovalTokens on_object_destroy_;
};

template <class T>
class DestructionFunctionsTokensPtrComparator {
public:
    using is_transparent = void;

    template <class TLhs, class TRhs>
    bool operator () (const TLhs& lhs, const TRhs& rhs) const {
        return get(lhs) == get(rhs);
    }
private:
    const T* get(const DestructionFunctionsTokensPtr<T>& v) const {
        return v.get();
    }
    const T* get(const DanglingBaseClassPtr<T>& v) const {
        return v.get();
    }
    const T* get(const DanglingBaseClassRef<T>& v) const {
        return &v.get();
    }
    const T* get(const T& v) const {
        return &v;
    }
};

}

namespace std {

template <class T>
struct hash<Mlib::DestructionFunctionsTokensPtr<T>>
{
    using is_transparent = void;

    std::size_t operator()(const Mlib::DestructionFunctionsTokensPtr<T>& s) const noexcept {
        return std::hash<T*>()(s.get());
    }
    std::size_t operator()(const Mlib::DanglingBaseClassPtr<T>& s) const noexcept {
        return std::hash<T*>()(s.get());
    }
    std::size_t operator()(const Mlib::DanglingBaseClassRef<T>& s) const noexcept {
        return std::hash<T*>()(&s.get());
    }
    std::size_t operator()(const T& s) const noexcept {
        return std::hash<T*>()(&s);
    }
};

}
