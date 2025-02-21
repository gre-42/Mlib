#include "Thread_Top_Logic.hpp"
#include <Mlib/Geometry/Cameras/Camera.hpp>
#include <Mlib/Layout/ILayout_Pixels.hpp>
#include <Mlib/Layout/Layout_Constraint_Parameters.hpp>
#include <Mlib/Layout/Screen_Units.hpp>
#include <Mlib/Log.hpp>
#include <Mlib/Render/Render_Setup.hpp>
#include <Mlib/Render/Text/Align_Text.hpp>
#include <Mlib/Render/Text/Renderable_Text.hpp>
#include <Mlib/Render/Text/Text_Interpolation_Mode.hpp>
#include <Mlib/Scene_Graph/Focus.hpp>
#include <Mlib/Scene_Graph/Focus_Filter.hpp>
#include <Mlib/Threads/Thread_Top.hpp>

using namespace Mlib;

ThreadTopLogic::ThreadTopLogic(
    VariableAndHash<std::string> charset,
    std::string ttf_filename,
    const FixedArray<float, 3>& color,
    const FixedArray<float, 2>& position,
    const ILayoutPixels& font_height,
    const ILayoutPixels& line_distance,
    std::chrono::milliseconds update_interval,
    Focus focus_mask)
    : RenderTextLogic{
        std::move(charset),
        std::move(ttf_filename),
        color,
        font_height,
        line_distance }
    , position_{ position }
    , last_update_{ std::chrono::steady_clock::time_point() }
    , update_interval_{ update_interval }
    , focus_mask_{ focus_mask }
{}

ThreadTopLogic::~ThreadTopLogic() {
    on_destroy.clear();
}

std::optional<RenderSetup> ThreadTopLogic::try_render_setup(
    const LayoutConstraintParameters& lx,
    const LayoutConstraintParameters& ly,
    const RenderedSceneDescriptor& frame_id) const
{
    return std::nullopt;
}

void ThreadTopLogic::render_without_setup(
    const LayoutConstraintParameters& lx,
    const LayoutConstraintParameters& ly,
    const RenderConfig& render_config,
    const SceneGraphConfig& scene_graph_config,
    RenderResults* render_results,
    const RenderedSceneDescriptor& frame_id)
{
    auto now = std::chrono::steady_clock::now();
    auto elapsed = now - last_update_;
    if ((last_update_ == std::chrono::steady_clock::time_point()) ||
        (elapsed > update_interval_))
    {
        text_ = thread_top();
        last_update_ = now;
    }
    LOG_FUNCTION("FocusedTextLogic::render");
    renderable_text().render(
        font_height_.to_pixels(ly, PixelsRoundMode::ROUND),
        position_,
        { lx.flength(), ly.flength() },
        text_,
        AlignText::TOP,
        TextInterpolationMode::NEAREST_NEIGHBOR,
        line_distance_.to_pixels(ly, PixelsRoundMode::NONE));
}

FocusFilter ThreadTopLogic::focus_filter() const {
    return { .focus_mask = focus_mask_ };
}

void ThreadTopLogic::print(std::ostream& ostr, size_t depth) const {
    ostr << std::string(depth, ' ') << "ThreadTopLogic (" << focus_to_string(focus_mask_) << ")\n";
}
