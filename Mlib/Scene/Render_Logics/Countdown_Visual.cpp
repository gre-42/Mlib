#include "Countdown_Visual.hpp"
#include <Mlib/Geometry/Cameras/Camera.hpp>
#include <Mlib/Layout/ILayout_Pixels.hpp>
#include <Mlib/Layout/Layout_Constraint_Parameters.hpp>
#include <Mlib/Layout/Screen_Units.hpp>
#include <Mlib/Log.hpp>
#include <Mlib/Macro_Executor/Expression_Watcher.hpp>
#include <Mlib/Memory/Dangling_Unique_Ptr.hpp>
#include <Mlib/Memory/Object_Pool.hpp>
#include <Mlib/Physics/Advance_Times/Countdown_Physics.hpp>
#include <Mlib/Render/Render_Config.hpp>
#include <Mlib/Render/Render_Setup.hpp>
#include <Mlib/Render/Text/Align_Text.hpp>
#include <Mlib/Render/Text/Charsets.hpp>
#include <Mlib/Render/Text/Renderable_Text.hpp>
#include <Mlib/Render/Text/Text_Interpolation_Mode.hpp>
#include <Mlib/Variable_And_Hash.hpp>

using namespace Mlib;

CountdownVisual::CountdownVisual(
    const DanglingBaseClassRef<CountdownPhysics>& physics,
    std::unique_ptr<ExpressionWatcher>&& ew,
    std::string charset,
    std::string ttf_filename,
    const FixedArray<float, 3>& color,
    const FixedArray<float, 2>& position,
    const ILayoutPixels& font_height,
    const ILayoutPixels& line_distance,
    std::string text)
    : RenderTextLogic{
        ascii,
        std::move(ttf_filename),
        color,
        font_height,
        line_distance }
    , on_destroy_countdown_physics_{ physics->on_destroy, CURRENT_SOURCE_LOCATION }
    , physics_{ physics }
    , ew_{ std::move(ew) }
    , charset_{ std::move(charset) }
    , text_{ std::move(text) }
    , position_{ position }
{
    on_destroy_countdown_physics_.add(
        [this](){ global_object_pool.remove(this); },
        CURRENT_SOURCE_LOCATION);
}

CountdownVisual::~CountdownVisual() {
    on_destroy.clear();
}

std::optional<RenderSetup> CountdownVisual::try_render_setup(
    const LayoutConstraintParameters& lx,
    const LayoutConstraintParameters& ly,
    const RenderedSceneDescriptor& frame_id) const
{
    return std::nullopt;
}

void CountdownVisual::render_without_setup(
    const LayoutConstraintParameters& lx,
    const LayoutConstraintParameters& ly,
    const RenderConfig& render_config,
    const SceneGraphConfig& scene_graph_config,
    RenderResults* render_results,
    const RenderedSceneDescriptor& frame_id)
{
    LOG_FUNCTION("CountdownVisual::render");
    auto seconds_remaining = physics_->seconds_remaining();
    if (seconds_remaining == 0) {
        return;
    }
    if (ew_->result_may_have_changed()) {
        renderable_text().set_charset(VariableAndHash{ew_->eval<std::string>(charset_)});
    }
    renderable_text().render(
        font_height_.to_pixels(ly, PixelsRoundMode::ROUND),
        position_,
        {lx.flength(), ly.flength()},
        text_.empty()
            ? std::to_string(seconds_remaining)
            : text_,
        VerticalTextAlignment::BOTTOM,
        TextInterpolationMode::NEAREST_NEIGHBOR,
        line_distance_.to_pixels(ly, PixelsRoundMode::NONE));
}

void CountdownVisual::print(std::ostream& ostr, size_t depth) const {
    ostr << std::string(depth, ' ') << "CountdownVisual";
}
