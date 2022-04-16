#include "Standard_Camera_Logic.hpp"
#include <Mlib/Fps/Set_Fps.hpp>
#include <Mlib/Log.hpp>
#include <Mlib/Math/Fixed_Math.hpp>
#include <Mlib/Render/CHK.hpp>
#include <Mlib/Render/Rendered_Scene_Descriptor.hpp>
#include <Mlib/Render/Selected_Cameras.hpp>
#include <Mlib/Scene_Graph/Containers/Scene.hpp>
#include <Mlib/Scene_Graph/Delete_Node_Mutex.hpp>
#include <Mlib/Scene_Graph/Elements/Scene_Node.hpp>
#include <Mlib/Scene_Graph/Scene_Graph_Config.hpp>

using namespace Mlib;

StandardCameraLogic::StandardCameraLogic(
    const Scene& scene,
    SelectedCameras& cameras,
    const DeleteNodeMutex& delete_node_mutex)
: scene_{ scene },
  cameras_{ cameras },
  delete_node_mutex_{ delete_node_mutex },
  camera_node_{ nullptr }
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
    if ((width == 0) || (height == 0)) {
        throw std::runtime_error("StandardCameraLogic::render received zero width or height");
    }
    float aspect_ratio = width / (float) height;

    if (!delete_node_mutex_.is_locked_by_this_thread()) {
        throw std::runtime_error("Deletion mutex not locked in StandardCameraLogic::render");
    }
    if ((frame_id.external_render_pass.pass == ExternalRenderPassType::LIGHTMAP_GLOBAL_STATIC) ||
        (frame_id.external_render_pass.pass == ExternalRenderPassType::LIGHTMAP_GLOBAL_DYNAMIC) ||
        (frame_id.external_render_pass.pass == ExternalRenderPassType::LIGHTMAP_LOCAL_INSTANCES_STATIC) ||
        (frame_id.external_render_pass.pass == ExternalRenderPassType::LIGHTMAP_NODE_DYNAMIC))
    {
        camera_node_ = &scene_.get_node(frame_id.light_node_name);
    } else if (frame_id.external_render_pass.pass == ExternalRenderPassType::DIRTMAP) {
        camera_node_ = &scene_.get_node(cameras_.dirtmap_node_name);
    } else if (frame_id.external_render_pass.pass == ExternalRenderPassType::STANDARD) {
        camera_node_ = &scene_.get_node(cameras_.camera_node_name());
    } else {
        throw std::runtime_error("StandardCameraLogic::render: unknown render pass");
    }
    auto camera_object = camera_node_->get_camera().copy();
    camera_object->set_aspect_ratio(aspect_ratio);
    vp_ = dot2d(
        camera_object->projection_matrix(),
        camera_node_->absolute_view_matrix().affine());
    iv_ = camera_node_->absolute_model_matrix();
}

float StandardCameraLogic::near_plane() const {
    return scene_.get_node(cameras_.camera_node_name()).get_camera().get_near_plane();
}

float StandardCameraLogic::far_plane() const {
    return scene_.get_node(cameras_.camera_node_name()).get_camera().get_far_plane();
}

const FixedArray<float, 4, 4>& StandardCameraLogic::vp() const {
    if (camera_node_ == nullptr) {
        throw std::runtime_error("camera node not set in StandardCameraLogic::vp");
    }
    return vp_;
}

const TransformationMatrix<float, 3>& StandardCameraLogic::iv() const {
    if (camera_node_ == nullptr) {
        throw std::runtime_error("camera node not set in StandardCameraLogic::iv");
    }
    return iv_;
}

const SceneNode& StandardCameraLogic::camera_node() const {
    if (camera_node_ == nullptr) {
        throw std::runtime_error("camera node not set in StandardCameraLogic::camera_node");
    }
    return *camera_node_;
}

bool StandardCameraLogic::requires_postprocessing() const {
    return scene_.get_node(cameras_.camera_node_name()).get_camera().get_requires_postprocessing();
}

void StandardCameraLogic::print(std::ostream& ostr, size_t depth) const {
    ostr << std::string(depth, ' ') << "StandardCameraLogic\n";
}
