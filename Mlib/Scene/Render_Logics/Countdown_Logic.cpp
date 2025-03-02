#include "Countdown_Logic.hpp"
#include <Mlib/Geometry/Cameras/Camera.hpp>
#include <Mlib/Layout/ILayout_Pixels.hpp>
#include <Mlib/Layout/Layout_Constraint_Parameters.hpp>
#include <Mlib/Layout/Screen_Units.hpp>
#include <Mlib/Log.hpp>
#include <Mlib/Macro_Executor/Expression_Watcher.hpp>
#include <Mlib/Macro_Executor/Focus.hpp>
#include <Mlib/Memory/Dangling_Unique_Ptr.hpp>
#include <Mlib/Physics/Units.hpp>
#include <Mlib/Render/Render_Config.hpp>
#include <Mlib/Render/Render_Setup.hpp>
#include <Mlib/Render/Text/Align_Text.hpp>
#include <Mlib/Render/Text/Charsets.hpp>
#include <Mlib/Render/Text/Renderable_Text.hpp>
#include <Mlib/Render/Text/Text_Interpolation_Mode.hpp>
#include <Mlib/Variable_And_Hash.hpp>
#include <mutex>

using namespace Mlib;

CountDownLogic::CountDownLogic(
    DanglingRef<SceneNode> node,
    std::unique_ptr<ExpressionWatcher>&& ew,
    std::string charset,
    std::string ttf_filename,
    const FixedArray<float, 3>& color,
    const FixedArray<float, 2>& position,
    const ILayoutPixels& font_height,
    const ILayoutPixels& line_distance,
    float duration,
    Focus pending_focus,
    Focus counting_focus,
    std::string text,
    Focuses& focuses)
    : RenderTextLogic{
        ascii,
        std::move(ttf_filename),
        color,
        font_height,
        line_distance }
    , on_node_clear{ node->on_clear, CURRENT_SOURCE_LOCATION }
    , node_{ node.ptr() }
    , ew_{ std::move(ew) }
    , charset_{ std::move(charset) }
    , elapsed_time_{ NAN }
    , duration_{ duration }
    , pending_focus_{ pending_focus }
    , counting_focus_{ counting_focus }
    , text_{ std::move(text) }
    , focuses_{ focuses }
    , position_{ position }
{}

CountDownLogic::~CountDownLogic() {
    on_destroy.clear();
    node_ = nullptr;
}

std::optional<RenderSetup> CountDownLogic::try_render_setup(
    const LayoutConstraintParameters& lx,
    const LayoutConstraintParameters& ly,
    const RenderedSceneDescriptor& frame_id) const
{
    return std::nullopt;
}

void CountDownLogic::render_without_setup(
    const LayoutConstraintParameters& lx,
    const LayoutConstraintParameters& ly,
    const RenderConfig& render_config,
    const SceneGraphConfig& scene_graph_config,
    RenderResults* render_results,
    const RenderedSceneDescriptor& frame_id)
{
    LOG_FUNCTION("CountDownLogic::render");
    bool counting;
    float elapsed;
    {
        std::shared_lock lock{focuses_.mutex};
        counting = focuses_.has_focus(Focus::QUERY_CONTAINS | counting_focus_);
        elapsed = elapsed_time_;
    }
    if (counting) {
        if (std::isnan(elapsed)) {
            THROW_OR_ABORT("Countdown elapsed time is NAN (0)");
        }
        if (ew_->result_may_have_changed()) {
            renderable_text().set_charset(VariableAndHash{ew_->eval<std::string>(charset_)});
        }
        renderable_text().render(
            font_height_.to_pixels(ly, PixelsRoundMode::ROUND),
            position_,
            {lx.flength(), ly.flength()},
            text_.empty()
                ? std::to_string((unsigned int)std::ceil((duration_ - elapsed) / seconds))
                : text_,
            VerticalTextAlignment::BOTTOM,
            TextInterpolationMode::NEAREST_NEIGHBOR,
            line_distance_.to_pixels(ly, PixelsRoundMode::NONE));
    }
}

void CountDownLogic::advance_time(float dt, const StaticWorld& world) {
    std::scoped_lock lock{focuses_.mutex};
    if (focuses_.has_focus(Focus::QUERY_CONTAINS | pending_focus_)) {
        elapsed_time_ = 0.f;
        focuses_.replace(pending_focus_, counting_focus_);
    }
    if (focuses_.has_focus(Focus::QUERY_CONTAINS | counting_focus_)) {
        if (std::isnan(elapsed_time_)) {
            THROW_OR_ABORT("Countdown elapsed time is NAN (1)");
        }
        if (focuses_.has_focus(counting_focus_)) {
            elapsed_time_ += dt;
        }
        if (elapsed_time_ >= duration_) {
            focuses_.remove(counting_focus_);
            elapsed_time_ = NAN;
        }
    }
}

void CountDownLogic::print(std::ostream& ostr, size_t depth) const {
    ostr << std::string(depth, ' ') << "CountDownLogic\n";
}
