#include "Move_Scene_Logic.hpp"
#include <Mlib/Log.hpp>
#include <Mlib/Render/CHK.hpp>
#include <Mlib/Render/Render_Config.hpp>
#include <Mlib/Render/Rendered_Scene_Descriptor.hpp>
#include <Mlib/Scene_Graph/Scene.hpp>

using namespace Mlib;

MoveSceneLogic::MoveSceneLogic(Scene& scene)
: scene_{scene}
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

    scene_.move(render_config.dt);
}

float MoveSceneLogic::near_plane() const {
    throw std::runtime_error("MoveSceneLogic::near_plane not implemented");
}

float MoveSceneLogic::far_plane() const {
    throw std::runtime_error("MoveSceneLogic::far_plane not implemented");
}

const FixedArray<float, 4, 4>& MoveSceneLogic::vp() const {
    throw std::runtime_error("MoveSceneLogic::vp not implemented");
}

const FixedArray<float, 4, 4>& MoveSceneLogic::iv() const {
    throw std::runtime_error("MoveSceneLogic::iv not implemented");
}

bool MoveSceneLogic::requires_postprocessing() const {
    throw std::runtime_error("MoveSceneLogic::requires_postprocessing not implemented");
}
