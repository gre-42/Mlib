#include "Visual_Movable_Logger.hpp"
#include <Mlib/Geometry/Cameras/Camera.hpp>
#include <Mlib/Log.hpp>
#include <Mlib/Macro_Executor/Focus.hpp>
#include <Mlib/Memory/Dangling_Unique_Ptr.hpp>
#include <Mlib/Memory/Object_Pool.hpp>
#include <Mlib/Physics/Containers/Advance_Times.hpp>
#include <Mlib/Players/Advance_Times/Player.hpp>
#include <Mlib/Render/Render_Logics/Render_Logics.hpp>
#include <Mlib/Render/Render_Setup.hpp>
#include <Mlib/Scene/Render_Logics/Visual_Movable_Logger_View.hpp>
#include <Mlib/Scene_Graph/Elements/Scene_Node.hpp>
#include <Mlib/Source_Location.hpp>
#include <ostream>

using namespace Mlib;

VisualMovableLogger::VisualMovableLogger(
    ObjectPool& object_pool,
    AdvanceTimes& advance_times,
    RenderLogics& render_logics,
    DanglingRef<SceneNode> node,
    DanglingBaseClassPtr<Player> player,
    FocusFilter focus_filter)
    : player_{ player }
    , node_{ node }
    , advance_times_ { advance_times }
    , render_logics_{ render_logics }
    , on_node_clear_{ node->on_clear, CURRENT_SOURCE_LOCATION }
    , on_player_delete_vehicle_internals_{ player != nullptr ? &player->delete_vehicle_internals : nullptr, CURRENT_SOURCE_LOCATION }
    , focus_filter_{ std::move(focus_filter) }
{
    advance_times_.add_advance_time({ *this, CURRENT_SOURCE_LOCATION }, CURRENT_SOURCE_LOCATION);
    if (!on_player_delete_vehicle_internals_.is_null()) {
        on_player_delete_vehicle_internals_.add([this, &op=object_pool]() { op.remove(this); }, CURRENT_SOURCE_LOCATION);
    }
    on_node_clear_.add([this, &op=object_pool]() { op.remove(this); }, CURRENT_SOURCE_LOCATION);
    render_logics_.append({ *this, CURRENT_SOURCE_LOCATION }, 0 /* z_order */, CURRENT_SOURCE_LOCATION);
}

VisualMovableLogger::~VisualMovableLogger() {
    on_destroy.clear();
}

void VisualMovableLogger::add_logger(std::unique_ptr<VisualMovableLoggerView>&& logger) {
    loggers_.push_back(std::move(logger));
}

void VisualMovableLogger::notify_destroyed(SceneNode& destroyed_object) {
    global_object_pool.remove(this);
}

void VisualMovableLogger::advance_time(float dt, const StaticWorld& world) {
    for (auto& l : loggers_) {
        l->advance_time(dt, world);
    }
}

std::optional<RenderSetup> VisualMovableLogger::try_render_setup(
    const LayoutConstraintParameters& lx,
    const LayoutConstraintParameters& ly,
    const RenderedSceneDescriptor& frame_id) const
{
    return std::nullopt;
}

void VisualMovableLogger::render_without_setup(
    const LayoutConstraintParameters& lx,
    const LayoutConstraintParameters& ly,
    const RenderConfig& render_config,
    const SceneGraphConfig& scene_graph_config,
    RenderResults* render_results,
    const RenderedSceneDescriptor& frame_id)
{
    LOG_FUNCTION("VisualMovableLogger::render");
    for (auto& l : loggers_) {
        l->render(
            lx,
            ly,
            render_config,
            scene_graph_config,
            render_results,
            frame_id);
    }
}

bool VisualMovableLogger::is_visible(const UiFocus& ui_focus) const {
    std::shared_lock lock{ ui_focus.focuses.mutex };
    return ui_focus.has_focus(focus_filter_);
}

void VisualMovableLogger::print(std::ostream& ostr, size_t depth) const {
    ostr << std::string(depth, ' ') << "VisualMovableLogger\n";
}
