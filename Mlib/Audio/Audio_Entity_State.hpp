#pragma once
#include <Mlib/Math/Transformation/Transformation_Matrix.hpp>
#include <Mlib/Scene_Config/Scene_Precision.hpp>

namespace Mlib {

struct AudioListenerState {
    TransformationMatrix<float, ScenePos, 3> pose;
    FixedArray<float, 3> velocity;
};

template <class TPosition>
struct AudioSourceState {
    FixedArray<TPosition, 3> position;
    FixedArray<float, 3> velocity;
};

}
