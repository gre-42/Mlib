#include "Blended.hpp"
#include <Mlib/Scene_Graph/Elements/Renderable.hpp>

using namespace Mlib;

Blended::Blended(
    std::shared_ptr<const RenderableWithStyle> renderable_with_style,
    const VariableAndHash<std::string>& name,
    const FixedArray<ScenePos, 4, 4>& mvp,
    const TransformationMatrix<float, ScenePos, 3>& m,
    std::shared_ptr<const AnimationState> animation_state,
    const std::list<std::shared_ptr<const ColorStyle>>& color_styles)
    : mvp{ mvp }
    , m{ m }
    , animation_state{ std::move(animation_state) }
    , color_style{ renderable_with_style->style(color_styles, name) }
    , z_order{ (*renderable_with_style)->continuous_blending_z_order() }
    , renderable_with_style_{ std::move(renderable_with_style) }
{ }

Blended::~Blended() = default;
