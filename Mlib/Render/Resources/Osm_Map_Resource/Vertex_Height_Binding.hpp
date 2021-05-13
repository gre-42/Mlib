#pragma once
#include <Mlib/Math/Fixed_Math.hpp>
#include <Mlib/Math/Orderable_Fixed_Array.hpp>
#include <Mlib/Strings/String.hpp>
#include <cmath>
#include <compare>
#include <stdexcept>

namespace Mlib {

template <typename TData, size_t... tshape>
class FixedArray;

class VertexHeightBinding {
public:
    VertexHeightBinding()
    : value_{ NAN, NAN }
    {}
    VertexHeightBinding& operator = (const FixedArray<float, 2>& v) {
        if (!is_undefined()) {
            throw std::runtime_error("Height binding already set");
        }
        if (any(Mlib::isnan(v))) {
            throw std::runtime_error("Height binding value forbidden");
        }
        value_ = v;
        return *this;
    }
    std::strong_ordering operator <=> (const FixedArray<float, 2>& v) const {
        if (is_undefined()) {
            throw std::runtime_error("Height binding undefiend");
        }
        return OrderableFixedArray{ value_ } <=> OrderableFixedArray{ v };
    }
    const FixedArray<float, 2>& value() const {
        return value_;
    }
    FixedArray<float, 2>& value() {
        return value_;
    }
private:
    bool is_undefined() const {
        return all(Mlib::isnan(value_));
    }
    FixedArray<float, 2> value_;
};

}
