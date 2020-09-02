#include "Standard_Camera_Logic.hpp"
#include <Mlib/Log.hpp>
#include <Mlib/Math/Fixed_Math.hpp>
#include <Mlib/Render/CHK.hpp>
#include <Mlib/Render/Render_Config.hpp>
#include <Mlib/Render/Rendered_Scene_Descriptor.hpp>
#include <Mlib/Render/Selected_Cameras.hpp>
#include <Mlib/Scene_Graph/Scene.hpp>
#include <Mlib/Scene_Graph/Scene_Graph_Config.hpp>
#include <Mlib/Set_Fps.hpp>

using namespace Mlib;

StandardCameraLogic::StandardCameraLogic(
    const Scene& scene,
    SelectedCameras& cameras)
: scene_{scene},
  cameras_{cameras}
{}

void StandardCameraLogic::update_projection_and_inverse_view_matrix(
    int width,
    int height,
    const RenderedSceneDescriptor& frame_id)
{
    float aspect_ratio = width / (float) height;

    SceneNode* cn;
    if (frame_id.external_render_pass.pass == ExternalRenderPass::LIGHTMAP_TO_TEXTURE) {
        cn = scene_.get_node(cameras_.light_node_names.at(frame_id.light_resource_id));
    } else if (frame_id.external_render_pass.pass == ExternalRenderPass::DIRTMAP) {
        cn = scene_.get_node(cameras_.dirtmap_node_name);
    } else {
        cn = scene_.get_node(cameras_.camera_node_name);
    }
    auto co = cn->get_camera()->copy();
    co->set_aspect_ratio(aspect_ratio);
    vp_ = dot2d(
        co->projection_matrix(),
        cn->absolute_view_matrix());
    iv_ = cn->absolute_model_matrix();
}

void StandardCameraLogic::render(
    int width,
    int height,
    const RenderConfig& render_config,
    const SceneGraphConfig& scene_graph_config,
    RenderResults* render_results,
    const RenderedSceneDescriptor& frame_id)
{
    LOG_FUNCTION("StandardCameraLogic::render");
    update_projection_and_inverse_view_matrix(width, height, frame_id);

    render_config.apply();

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

    scene_.render(vp_, iv_, render_config, scene_graph_config, frame_id.external_render_pass);

    render_config.unapply();
}

float StandardCameraLogic::near_plane() const {
    return scene_.get_node(cameras_.camera_node_name)->get_camera()->get_near_plane();
}

float StandardCameraLogic::far_plane() const {
    return scene_.get_node(cameras_.camera_node_name)->get_camera()->get_far_plane();
}

const FixedArray<float, 4, 4>& StandardCameraLogic::vp() const {
    return vp_;
}

bool StandardCameraLogic::requires_postprocessing() const {
    return scene_.get_node(cameras_.camera_node_name)->get_camera()->get_requires_postprocessing();
}
