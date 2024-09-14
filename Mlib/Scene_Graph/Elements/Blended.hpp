#pragma once
#include <Mlib/Array/Fixed_Array.hpp>
#include <Mlib/Math/Transformation/Transformation_Matrix.hpp>
#include <Mlib/Scene_Pos.hpp>

namespace Mlib {

struct AnimationState;
struct ColorStyle;
class Renderable;

struct Blended {
    int z_order;
    FixedArray<ScenePos, 4, 4> mvp;
    TransformationMatrix<float, ScenePos, 3> m;
    const Renderable* renderable;
    const AnimationState* animation_state;
    const ColorStyle* color_style;
    inline std::pair<int, ScenePos> sorting_key() const {
        return { z_order, mvp(2, 3) };
    }
};

}
