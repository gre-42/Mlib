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
struct DestructionFunctionsTokensPtrComparator {
    using is_transparent = void;

    bool operator () (const DestructionFunctionsTokensPtr<T>& lhs, const DestructionFunctionsTokensPtr<T>& rhs) const {
        return lhs.get() == rhs.get();
    }
    bool operator () (const DestructionFunctionsTokensPtr<T>& lhs, const DanglingBaseClassPtr<T>& rhs) const {
        return lhs.get() == rhs.get();
    }
    bool operator () (const DanglingBaseClassPtr<T>& rhs, const DestructionFunctionsTokensPtr<T>& lhs) const {
        return lhs.get() == rhs.get();
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
};

}
