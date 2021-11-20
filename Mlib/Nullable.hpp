#pragma once
#include <ostream>

namespace Mlib {

template <class TData>
struct Nullable {
    Nullable(TData* data)
    : data{ data }
    {}
    operator TData* () {
        return data;
    }
    operator const TData* () const {
        return data;
    }
    TData* data;
};

template <class TData>
bool operator == (const Nullable<TData>& a, const Nullable<TData>& b) {
    if ((a.data == nullptr) && (b.data == nullptr)) {
        return true;
    }
    if ((a.data == nullptr) != (b.data == nullptr)) {
        return false;
    }
    return *a.data == *b.data;
}

template <class TData>
bool operator == (const TData* a, const Nullable<TData>& b) {
    if ((a == nullptr) && (b.data == nullptr)) {
        return true;
    }
    if ((a == nullptr) != (b.data == nullptr)) {
        return false;
    }
    return *a == *b.data;
}

template <class TData>
std::ostream& operator << (std::ostream& ostr, const Nullable<TData>& v) {
    if (v == nullptr) {
        ostr << "null";
    } else {
        ostr << *v;
    }
    return ostr;
}

}
