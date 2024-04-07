#pragma once

namespace Mlib {

enum class ApplyOverAxisType {
    REDUCE,
    NOREDUCE
};

template <class TDerived, class TData>
class BaseDenseArray {
public:
    constexpr const TDerived* operator -> () const {
        return reinterpret_cast<const TDerived*>(this);
    }
    constexpr TDerived* operator -> () {
        return reinterpret_cast<TDerived*>(this);
    }
    constexpr const TDerived& operator * () const {
        return *reinterpret_cast<const TDerived*>(this);
    }
    constexpr TDerived& operator * () {
        return *reinterpret_cast<TDerived*>(this);
    }
};

}
