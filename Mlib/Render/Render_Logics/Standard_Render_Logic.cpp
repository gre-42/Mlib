#include "Standard_Render_Logic.hpp"
#include <Mlib/Log.hpp>
#include <Mlib/Render/CHK.hpp>
#include <Mlib/Render/Render_Config.hpp>
#include <Mlib/Render/Rendered_Scene_Descriptor.hpp>
#include <Mlib/Scene_Graph/Scene.hpp>

using namespace Mlib;

StandardRenderLogic::StandardRenderLogic(
    const Scene& scene,
    RenderLogic& skybox_logic)
: scene_{scene},
  skybox_logic_{skybox_logic}
{}

void StandardRenderLogic::render(
    int width,
    int height,
    const RenderConfig& render_config,
    const SceneGraphConfig& scene_graph_config,
    RenderResults* render_results,
    const RenderedSceneDescriptor& frame_id)
{
    LOG_FUNCTION("StandardRenderLogic::render");

    // make sure we clear the framebuffer's content
    if (frame_id.external_render_pass.pass == ExternalRenderPass::LIGHTMAP_TO_TEXTURE) {
        CHK(glClearColor(1.f, 1.f, 1.f, 1.f));
    } else {
        CHK(glClearColor(
            render_config.background_color(0),
            render_config.background_color(1),
            render_config.background_color(2),
            1));
    }
    CHK(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));

    skybox_logic_.render(width, height, render_config, scene_graph_config, render_results, frame_id);

    render_config.apply();

    scene_.render(skybox_logic_.vp(), skybox_logic_.iv(), render_config, scene_graph_config, frame_id.external_render_pass);

    render_config.unapply();
}

float StandardRenderLogic::near_plane() const {
    return skybox_logic_.near_plane();
}

float StandardRenderLogic::far_plane() const {
    return skybox_logic_.far_plane();
}

const FixedArray<float, 4, 4>& StandardRenderLogic::vp() const {
    return skybox_logic_.vp();
}

const FixedArray<float, 4, 4>& StandardRenderLogic::iv() const {
    return skybox_logic_.iv();
}

bool StandardRenderLogic::requires_postprocessing() const {
    return skybox_logic_.requires_postprocessing();
}
