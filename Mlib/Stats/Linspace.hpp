#pragma once
#include <Mlib/Math/Math.hpp>

namespace Mlib {

template <class TData>
class Linspace {
public:
    Linspace(const TData& from, const TData& to, size_t count)
    : from_{from}, to_{to}, count_{count}
    {}
    TData operator [] (size_t i) const {
        return (from_ * (count_ - i - 1) + to_ * i) / (count_ - 1);
    }
    size_t length() const {
        return count_;
    }
private:
    TData from_;
    TData to_;
    size_t count_;
};

template <class TData>
Array<TData> linspace(const TData& from, const TData& to, size_t count) {
    Array<TData> result{ArrayShape{count}};
    if (count == 1) {
        result = (from + to) / 2;
    } else {
        for (size_t i = 0; i < count; ++i) {
            result(i) = (from * (count - i - 1) + to * i) / (count - 1);
        }
    }
    return result;
}

}
