#include "Move_Scene_Logic.hpp"
#include <Mlib/Log.hpp>
#include <Mlib/Render/CHK.hpp>
#include <Mlib/Render/Render_Config.hpp>
#include <Mlib/Render/Rendered_Scene_Descriptor.hpp>
#include <Mlib/Scene_Graph/Containers/Scene.hpp>
#include <Mlib/Scene_Graph/Delete_Node_Mutex.hpp>

using namespace Mlib;

MoveSceneLogic::MoveSceneLogic(
    Scene& scene,
    DeleteNodeMutex& delete_node_mutex,
    float speed)
: scene_{ scene },
  deleter_thread_set_{ false },
  delete_node_mutex_{ delete_node_mutex },
  speed_{ speed }
{}

void MoveSceneLogic::render(
    int width,
    int height,
    float xdpi,
    float ydpi,
    const RenderConfig& render_config,
    const SceneGraphConfig& scene_graph_config,
    RenderResults* render_results,
    const RenderedSceneDescriptor& frame_id)
{
    LOG_FUNCTION("MoveSceneLogic::render");

    if (!deleter_thread_set_) {
        delete_node_mutex_.clear_deleter_thread();
        delete_node_mutex_.set_deleter_thread();
        deleter_thread_set_ = true;
    }
    scene_.move(render_config.min_dt * speed_);
}

void MoveSceneLogic::print(std::ostream& ostr, size_t depth) const {
    ostr << std::string(depth, ' ') << "MoveSceneLogic\n";
}
