#include "Standard_Camera_Logic.hpp"
#include <Mlib/Log.hpp>
#include <Mlib/Math/Fixed_Math.hpp>
#include <Mlib/Render/CHK.hpp>
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

void StandardCameraLogic::render(
    int width,
    int height,
    const RenderConfig& render_config,
    const SceneGraphConfig& scene_graph_config,
    RenderResults* render_results,
    const RenderedSceneDescriptor& frame_id)
{
    LOG_FUNCTION("StandardCameraLogic::render");
    float aspect_ratio = width / (float) height;

    SceneNode* cn;
    if (frame_id.external_render_pass.pass == ExternalRenderPass::LIGHTMAP_TO_TEXTURE) {
        cn = scene_.get_node(frame_id.light_node_name);
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

float StandardCameraLogic::near_plane() const {
    return scene_.get_node(cameras_.camera_node_name)->get_camera()->get_near_plane();
}

float StandardCameraLogic::far_plane() const {
    return scene_.get_node(cameras_.camera_node_name)->get_camera()->get_far_plane();
}

const FixedArray<float, 4, 4>& StandardCameraLogic::vp() const {
    return vp_;
}

const FixedArray<float, 4, 4>& StandardCameraLogic::iv() const {
    return iv_;
}

bool StandardCameraLogic::requires_postprocessing() const {
    return scene_.get_node(cameras_.camera_node_name)->get_camera()->get_requires_postprocessing();
}
