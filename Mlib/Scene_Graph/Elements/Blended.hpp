#pragma once
#include <Mlib/Array/Fixed_Array.hpp>
#include <Mlib/Math/Transformation/Transformation_Matrix.hpp>
#include <Mlib/Scene_Config/Scene_Precision.hpp>
#include <Mlib/Scene_Graph/Elements/Animation_State.hpp>
#include <Mlib/Scene_Graph/Elements/Renderable_With_Style.hpp>
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
        std::shared_ptr<const AnimationState> animation_state,
        const std::list<std::shared_ptr<const ColorStyle>>& color_styles);
    ~Blended();
    FixedArray<ScenePos, 4, 4> mvp;
    TransformationMatrix<float, ScenePos, 3> m;
    std::shared_ptr<const AnimationState> animation_state;
    inline const Renderable& renderable() const {
        return *renderable_with_style_;
    }
    const ColorStyle* color_style;
    inline std::pair<int, ScenePos> sorting_key() const {
        return { z_order, -mvp(2, 3) };
    }
private:
    int z_order;
    std::shared_ptr<const RenderableWithStyle> renderable_with_style_;
};

}
