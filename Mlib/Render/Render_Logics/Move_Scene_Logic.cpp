#include "Move_Scene_Logic.hpp"
#include <Mlib/Log.hpp>
#include <Mlib/Render/CHK.hpp>
#include <Mlib/Render/Render_Config.hpp>
#include <Mlib/Render/Rendered_Scene_Descriptor.hpp>
#include <Mlib/Scene_Graph/Scene.hpp>

using namespace Mlib;

MoveSceneLogic::MoveSceneLogic(Scene& scene, float speed)
: scene_{scene},
  speed_{speed}
{}

void MoveSceneLogic::render(
    int width,
    int height,
    const RenderConfig& render_config,
    const SceneGraphConfig& scene_graph_config,
    RenderResults* render_results,
    const RenderedSceneDescriptor& frame_id)
{
    LOG_FUNCTION("MoveSceneLogic::render");

    scene_.move(render_config.dt * speed_);
}
