#pragma once
#include <Mlib/Math/Fixed_Math.hpp>
#include <Mlib/Math/Orderable_Fixed_Array.hpp>
#include <Mlib/Throw_Or_Abort.hpp>
#include <cmath>
#include <compare>
#include <stdexcept>

namespace Mlib {

template <typename TData, size_t... tshape>
class FixedArray;

template <class TPos>
class VertexHeightBinding {
public:
    VertexHeightBinding()
    : value_{ NAN, NAN }
    {}
    VertexHeightBinding& operator = (const FixedArray<TPos, 2>& v) {
        if (!is_undefined()) {
            THROW_OR_ABORT("Height binding already set");
        }
        if (any(Mlib::isnan(v))) {
            THROW_OR_ABORT("Height binding value forbidden");
        }
        value_ = v;
        return *this;
    }
    std::strong_ordering operator <=> (const FixedArray<TPos, 2>& v) const {
        if (is_undefined()) {
            THROW_OR_ABORT("Height binding undefined");
        }
        return OrderableFixedArray{ value_ } <=> OrderableFixedArray{ v };
    }
    const FixedArray<TPos, 2>& value() const {
        return value_;
    }
    FixedArray<TPos, 2>& value() {
        return value_;
    }
private:
    bool is_undefined() const {
        return all(Mlib::isnan(value_));
    }
    FixedArray<TPos, 2> value_;
};

}
