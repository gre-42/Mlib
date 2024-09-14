#include "Blended.hpp"
#include <Mlib/Scene_Graph/Elements/Renderable.hpp>

using namespace Mlib;

Blended::Blended(
    std::shared_ptr<const RenderableWithStyle> renderable_with_style,
    const std::string& name,
    const FixedArray<ScenePos, 4, 4>& mvp,
    const TransformationMatrix<float, ScenePos, 3>& m,
    const AnimationState* animation_state,
    const std::list<const ColorStyle*>& ecolor_styles)
    : z_order{ (*renderable_with_style)->continuous_blending_z_order() }
    , mvp{ mvp }
    , m{ m }
    , color_style{ renderable_with_style->style(ecolor_styles, name) }
    , renderable_with_style_{ std::move(renderable_with_style) }
    , animation_state_{ animation_state == nullptr ? std::nullopt : std::optional{ *animation_state } }
{ }

Blended::~Blended() = default;
