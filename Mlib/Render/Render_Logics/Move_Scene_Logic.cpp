#include "Move_Scene_Logic.hpp"
#include <Mlib/Geometry/Cameras/Camera.hpp>
#include <Mlib/Log.hpp>
#include <Mlib/Render/CHK.hpp>
#include <Mlib/Render/Render_Config.hpp>
#include <Mlib/Render/Render_Setup.hpp>
#include <Mlib/Render/Rendered_Scene_Descriptor.hpp>
#include <Mlib/Scene_Graph/Containers/Scene.hpp>
#include <Mlib/Scene_Graph/Delete_Node_Mutex.hpp>

using namespace Mlib;

MoveSceneLogic::MoveSceneLogic(
    Scene& scene,
    DeleteNodeMutex& delete_node_mutex,
    float speed)
: scene_{ scene },
  first_render_{ true },
  delete_node_mutex_{ delete_node_mutex },
  speed_{ speed }
{}

MoveSceneLogic::~MoveSceneLogic() {
    on_destroy.clear();
}

std::optional<RenderSetup> MoveSceneLogic::try_render_setup(
    const LayoutConstraintParameters& lx,
    const LayoutConstraintParameters& ly,
    const RenderedSceneDescriptor& frame_id) const
{
    return std::nullopt;
}

void MoveSceneLogic::render_without_setup(
    const LayoutConstraintParameters& lx,
    const LayoutConstraintParameters& ly,
    const RenderConfig& render_config,
    const SceneGraphConfig& scene_graph_config,
    RenderResults* render_results,
    const RenderedSceneDescriptor& frame_id)
{
    LOG_FUNCTION("MoveSceneLogic::render");

    auto time = std::chrono::steady_clock::now();
    if (first_render_) {
        last_time_ = time;
        delete_node_mutex_.clear_deleter_thread();
        delete_node_mutex_.set_deleter_thread();
        first_render_ = false;
    } else {
        scene_.move(std::chrono::duration<float>(time - last_time_).count() * speed_, time);
    }
}

void MoveSceneLogic::print(std::ostream& ostr, size_t depth) const {
    ostr << std::string(depth, ' ') << "MoveSceneLogic";
}
