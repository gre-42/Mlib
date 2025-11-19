#pragma once
#include <Mlib/Memory/Dangling_Base_Class.hpp>
#include <Mlib/Memory/Destruction_Functions.hpp>
#include <Mlib/Std_Hash.hpp>
#include <compare>
#include <type_traits>

namespace Mlib {

template <class T>
class DestructionFunctionsTokensRef {
    DestructionFunctionsTokensRef(const DestructionFunctionsTokensRef&) = delete;
    DestructionFunctionsTokensRef& operator = (const DestructionFunctionsTokensRef&) = delete;
public:
    using value_type = T;

    template <class TDerived>
        requires std::is_convertible_v<TDerived&, T&>
    inline DestructionFunctionsTokensRef(
        const DanglingBaseClassRef<TDerived>& o,
        DestructionFunctions& on_destroy,
        SourceLocation loc)
        : object_{ o }
        , on_object_destroy_{ on_destroy, loc }
    {}
    template <class TDerived>
        requires std::is_convertible_v<TDerived&, T&>
    inline DestructionFunctionsTokensRef(const DanglingBaseClassRef<TDerived>& o, SourceLocation loc)
        : DestructionFunctionsTokensRef{ o, o->on_destroy, loc }
    {}
    inline T* operator -> () const {
        return &object_.get();
    }
    inline T& get() const {
        return object_.get();
    }
    inline const DanglingBaseClassRef<T>& object() const {
        return object_;
    }
    inline void on_destroy(std::function<void()> func, SourceLocation loc) const {
        on_object_destroy_.add(std::move(func), std::move(loc));
    }
    inline std::strong_ordering operator <=> (const DanglingBaseClassPtr<T>& other) const {
        return &object_.get() <=> other.get();
    }
    inline bool operator == (const DanglingBaseClassPtr<T>& other) const {
        return &object_.get() == other.get();
    }
    inline bool operator != (const DanglingBaseClassPtr<T>& other) const {
        return &object_.get() != other.get();
    }
private:
    DanglingBaseClassRef<T> object_;
    mutable DestructionFunctionsRemovalTokens on_object_destroy_;
};

}
