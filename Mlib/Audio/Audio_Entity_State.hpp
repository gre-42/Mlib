#pragma once
#include <Mlib/Math/Transformation/Transformation_Matrix.hpp>

namespace Mlib {

struct AudioListenerState {
    TransformationMatrix<float, double, 3> pose;
    FixedArray<float, 3> velocity;
};

template <class TPosition>
struct AudioSourceState {
    FixedArray<TPosition, 3> position;
    FixedArray<float, 3> velocity;
};

}
