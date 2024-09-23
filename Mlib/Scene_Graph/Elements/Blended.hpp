#pragma once
#include <Mlib/Array/Fixed_Array.hpp>
#include <Mlib/Math/Transformation/Transformation_Matrix.hpp>
#include <Mlib/Scene_Graph/Elements/Animation_State.hpp>
#include <Mlib/Scene_Graph/Elements/Renderable_With_Style.hpp>
#include <Mlib/Scene_Pos.hpp>
#include <memory>

namespace Mlib {

struct ColorStyle;
class RenderableWithStyle;
class Renderable;

class Blended {
    Blended(const Blended&) = delete;
    Blended& operator = (const Blended&) = delete;
public:
    Blended(
        std::shared_ptr<const RenderableWithStyle> renderable_with_style,
        const VariableAndHash<std::string>& name,
        const FixedArray<ScenePos, 4, 4>& mvp,
        const TransformationMatrix<float, ScenePos, 3>& m,
        const AnimationState* animation_state,
        const std::list<const ColorStyle*>& ecolor_styles);
    ~Blended();
    int z_order;
    FixedArray<ScenePos, 4, 4> mvp;
    TransformationMatrix<float, ScenePos, 3> m;
    inline const Renderable& renderable() const {
        return *renderable_with_style_;
    }
    inline const AnimationState* animation_state() const {
        return animation_state_.has_value() ? &*animation_state_ : nullptr;
    }
    const ColorStyle* color_style;
    inline std::pair<int, ScenePos> sorting_key() const {
        return { z_order, mvp(2, 3) };
    }
private:
    std::shared_ptr<const RenderableWithStyle> renderable_with_style_;
    const std::optional<AnimationState> animation_state_;
};

}
