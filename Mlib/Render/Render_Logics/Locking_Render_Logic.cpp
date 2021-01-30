#include "Locking_Render_Logic.hpp"

using namespace Mlib;

LockingRenderLogic::LockingRenderLogic(
    RenderLogic& child_logic,
    std::recursive_mutex& mutex)
: child_logic_{child_logic},
    mutex_{mutex}
{}

void LockingRenderLogic::render(
    int width,
    int height,
    const RenderConfig& render_config,
    const SceneGraphConfig& scene_graph_config,
    RenderResults* render_results,
    const RenderedSceneDescriptor& frame_id)
{
    // TimeGuard tg0{ "Render lock", "Render lock" };
    std::lock_guard lock{mutex_}; // formerly shared_lock
    // TimeGuard tg1{ "Locked render", "Locked render" };
    child_logic_.render(
        width,
        height,
        render_config,
        scene_graph_config,
        render_results,
        frame_id);
}
