#include "Visual_Movable_Logger.hpp"
#include <Mlib/Log.hpp>
#include <Mlib/Memory/Dangling_Unique_Ptr.hpp>
#include <Mlib/Physics/Containers/Advance_Times.hpp>
#include <Mlib/Scene/Render_Logics/Visual_Movable_Logger_View.hpp>
#include <ostream>

using namespace Mlib;

VisualMovableLogger::VisualMovableLogger(AdvanceTimes& advance_times)
: advance_times_{advance_times}
{}

VisualMovableLogger::~VisualMovableLogger() = default;

void VisualMovableLogger::add_logger(std::unique_ptr<VisualMovableLoggerView>&& logger) {
    loggers_.push_back(std::move(logger));
}

void VisualMovableLogger::notify_destroyed(DanglingRef<const SceneNode> destroyed_object) {
    advance_times_.delete_advance_time(*this, std::source_location::current());
}

void VisualMovableLogger::advance_time(float dt) {
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
