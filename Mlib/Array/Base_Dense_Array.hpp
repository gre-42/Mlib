#pragma once

enum class ApplyOverAxisType {
    REDUCE,
    NOREDUCE
};

template <class TDerived, class TData>
class BaseDenseArray {
public:
    const TDerived* operator -> () const {
        return reinterpret_cast<const TDerived*>(this);
    }
    TDerived* operator -> () {
        return reinterpret_cast<TDerived*>(this);
    }
    const TDerived& operator * () const {
        return *reinterpret_cast<const TDerived*>(this);
    }
    TDerived& operator * () {
        return *reinterpret_cast<TDerived*>(this);
    }
};
