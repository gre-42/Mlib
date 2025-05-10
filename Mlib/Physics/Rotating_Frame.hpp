#pragma once
#include <Mlib/Geometry/Fixed_Cross.hpp>
#include <Mlib/Math/Transformation/Transformation_Matrix.hpp>
#include <Mlib/Throw_Or_Abort.hpp>

namespace Mlib {

template <class TDir, class TPos, size_t n>
struct RotatingFrame {
    TransformationMatrix<TDir, TPos, n> location;
    FixedArray<TDir, n> v;
    FixedArray<TDir, n> w;
    FixedArray<TDir, n> velocity_at_position(const FixedArray<TPos, n>& position) const {
        if constexpr (n == 3) {
            using High = decltype(TDir() + TPos());
            auto wh = w.template casted<High>();
            auto dph = (position - location.t).template casted<High>();
            return v + cross(wh, dph).template casted<TDir>();
        } else {
            THROW_OR_ABORT("velocity_at_position not implemented for dimension \"" + std::to_string(n) + '"');
        }
    }
};

}
