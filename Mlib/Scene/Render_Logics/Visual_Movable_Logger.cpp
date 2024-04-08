#include "Visual_Movable_Logger.hpp"
#include <Mlib/Log.hpp>
#include <Mlib/Memory/Dangling_Unique_Ptr.hpp>
#include <Mlib/Physics/Containers/Advance_Times.hpp>
#include <Mlib/Players/Advance_Times/Player.hpp>
#include <Mlib/Render/Render_Logics/Render_Logics.hpp>
#include <Mlib/Scene/Render_Logics/Visual_Movable_Logger_View.hpp>
#include <Mlib/Source_Location.hpp>
#include <ostream>

using namespace Mlib;

VisualMovableLogger::VisualMovableLogger(
    AdvanceTimes& advance_times,
    RenderLogics& render_logics,
    DanglingRef<SceneNode> node,
    DanglingBaseClassPtr<Player> player)
    : shutting_down_{ false }
    , player_{ player }
    , node_{ node }
    , advance_times_ { advance_times }
    , render_logics_{ render_logics }
    , on_player_delete_externals_{ player != nullptr ? &player->delete_externals : nullptr }
{}

void VisualMovableLogger::init() {
    advance_times_.add_advance_time(*this);
    if (!on_player_delete_externals_.is_null()) {
        on_player_delete_externals_.add([this]() { if (!shutting_down_) { render_logics_.remove(*this); }});
    }
    render_logics_.append(node_.ptr(), shared_from_this(), 0 /* z_order */);
}

VisualMovableLogger::~VisualMovableLogger() {
    if (shutting_down_) {
        verbose_abort("VisualMovableLogger already shutting down");
    }
    shutting_down_ = true;
    advance_times_.delete_advance_time(*this, CURRENT_SOURCE_LOCATION);
}

void VisualMovableLogger::add_logger(std::unique_ptr<VisualMovableLoggerView>&& logger) {
    loggers_.push_back(std::move(logger));
}

void VisualMovableLogger::notify_destroyed(DanglingRef<SceneNode> destroyed_object) {
    advance_times_.delete_advance_time(*this, CURRENT_SOURCE_LOCATION);
}

void VisualMovableLogger::advance_time(float dt, std::chrono::steady_clock::time_point time) {
    for (auto& l : loggers_) {
        l->advance_time(dt);
    }
}

void VisualMovableLogger::render(
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

void VisualMovableLogger::print(std::ostream& ostr, size_t depth) const {
    ostr << std::string(depth, ' ') << "VisualMovableLogger\n";
}
