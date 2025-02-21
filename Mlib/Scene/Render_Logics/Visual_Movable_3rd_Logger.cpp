#include "Visual_Movable_3rd_Logger.hpp"
#include <Mlib/Geometry/Cameras/Camera.hpp>
#include <Mlib/Geometry/Coordinates/Homogeneous.hpp>
#include <Mlib/Layout/ILayout_Pixels.hpp>
#include <Mlib/Layout/Layout_Constraint_Parameters.hpp>
#include <Mlib/Layout/Screen_Units.hpp>
#include <Mlib/Log.hpp>
#include <Mlib/Math/Fixed_Math.hpp>
#include <Mlib/Memory/Object_Pool.hpp>
#include <Mlib/Physics/Containers/Advance_Times.hpp>
#include <Mlib/Render/Render_Logics/Render_Logics.hpp>
#include <Mlib/Render/Render_Setup.hpp>
#include <Mlib/Render/Text/Align_Text.hpp>
#include <Mlib/Render/Text/Renderable_Text.hpp>
#include <Mlib/Render/Text/Text_Interpolation_Mode.hpp>
#include <Mlib/Scene_Graph/Elements/Scene_Node.hpp>
#include <sstream>

using namespace Mlib;

VisualMovable3rdLogger::VisualMovable3rdLogger(
    RenderLogic& scene_logic,
    const DanglingRef<SceneNode>& scene_node,
    RenderLogics& render_logics,
    AdvanceTimes& advance_times,
    StatusWriter& status_writer,
    StatusComponents log_components,
    VariableAndHash<std::string> charset,
    std::string ttf_filename,
    const FixedArray<float, 2>& offset,
    const FixedArray<float, 3>& font_color,
    const ILayoutPixels& font_height,
    const ILayoutPixels& line_distance)
    : on_node_clear{ scene_node->on_clear, CURRENT_SOURCE_LOCATION }
    , scene_logic_{ scene_logic }
    , scene_node_{ scene_node.ptr() }
    , status_writer_{ status_writer }
    , log_components_{ log_components }
    , offset_{ offset }
    , font_color_{ font_color }
    , line_distance_{ line_distance }
    , charset_{ std::move(charset) }
    , ttf_filename_{ std::move(ttf_filename) }
    , font_height_{ font_height }
{
    on_node_clear.add([this]() { global_object_pool.remove(this); }, CURRENT_SOURCE_LOCATION);
    render_logics.append({ *this, CURRENT_SOURCE_LOCATION }, 0 /* z_order */, CURRENT_SOURCE_LOCATION);
    advance_times.add_advance_time({ *this, CURRENT_SOURCE_LOCATION }, CURRENT_SOURCE_LOCATION);
}

VisualMovable3rdLogger::~VisualMovable3rdLogger() {
    on_destroy.clear();
}

void VisualMovable3rdLogger::notify_destroyed(SceneNode& destroyed_object) {
    std::scoped_lock lock{ mutex_ };
    scene_node_ = nullptr;
}

void VisualMovable3rdLogger::advance_time(float dt, const StaticWorld& world) {
    std::stringstream sstr;
    status_writer_.write_status(sstr, log_components_, world);
    text_ = sstr.str();
}

std::optional<RenderSetup> VisualMovable3rdLogger::try_render_setup(
    const LayoutConstraintParameters& lx,
    const LayoutConstraintParameters& ly,
    const RenderedSceneDescriptor& frame_id) const
{
    return scene_logic_.try_render_setup(lx, ly, frame_id);
}

void VisualMovable3rdLogger::render_with_setup(
    const LayoutConstraintParameters& lx,
    const LayoutConstraintParameters& ly,
    const RenderConfig& render_config,
    const SceneGraphConfig& scene_graph_config,
    RenderResults* render_results,
    const RenderedSceneDescriptor& frame_id,
    const RenderSetup& setup)
{
    LOG_FUNCTION("VisualMovable3rdLogger::render");
    FixedArray<ScenePos, 3> node_pos = uninitialized;
    {
        std::scoped_lock lock{ mutex_ };
        if (scene_node_ == nullptr) {
            return;
        }
        node_pos = scene_node_->absolute_model_matrix().t;
    }
    if (renderable_text_ == nullptr) {
        renderable_text_ = std::make_unique<TextResource>(
            charset_,
            ttf_filename_,
            font_color_);
    }
    auto position4 = dot1d(setup.vp, homogenized_4(node_pos));
    if (position4(2) > setup.camera->get_near_plane()) {
        FixedArray<float, 2> position2{
            float(position4(0) / position4(3)) + offset_(0),
            -float(position4(1) / position4(3)) - offset_(1)};
        FixedArray<float, 2> size{lx.flength(), ly.flength()};
        auto p2 = (position2 * 0.5f + 0.5f) * size;
        renderable_text_->render(
            font_height_.to_pixels(ly, PixelsRoundMode::ROUND),
            p2,
            size,
            (std::string)text_,
            AlignText::BOTTOM,
            TextInterpolationMode::NEAREST_NEIGHBOR,
            line_distance_.to_pixels(ly, PixelsRoundMode::NONE));
    }
}

void VisualMovable3rdLogger::print(std::ostream& ostr, size_t depth) const {
    ostr << std::string(depth, ' ') << "VisualMovable3rdLogger\n";
}
