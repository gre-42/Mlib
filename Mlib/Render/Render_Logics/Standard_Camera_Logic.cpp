#include "Standard_Camera_Logic.hpp"
#include <Mlib/Geometry/Cameras/Camera.hpp>
#include <Mlib/Layout/Layout_Constraint_Parameters.hpp>
#include <Mlib/Log.hpp>
#include <Mlib/Math/Fixed_Math.hpp>
#include <Mlib/Render/CHK.hpp>
#include <Mlib/Render/Rendered_Scene_Descriptor.hpp>
#include <Mlib/Render/Selected_Cameras/Selected_Cameras.hpp>
#include <Mlib/Scene_Graph/Containers/Scene.hpp>
#include <Mlib/Scene_Graph/Delete_Node_Mutex.hpp>
#include <Mlib/Scene_Graph/Elements/Scene_Node.hpp>
#include <Mlib/Scene_Graph/Scene_Graph_Config.hpp>
#include <Mlib/Throw_Or_Abort.hpp>
#include <Mlib/Time/Fps/Set_Fps.hpp>

using namespace Mlib;

StandardCameraLogic::StandardCameraLogic(
    const Scene& scene,
    const SelectedCameras& cameras)
    : scene_{ scene }
    , cameras_{ cameras }
    , vp_{ fixed_nans<ScenePos, 4, 4>() }
    , iv_{ fixed_nans<ScenePos, 4, 4>() }
    , camera_node_{ nullptr }
{}

StandardCameraLogic::~StandardCameraLogic() {
    on_destroy.clear();
}

void StandardCameraLogic::render(
    const LayoutConstraintParameters& lx,
    const LayoutConstraintParameters& ly,
    const RenderConfig& render_config,
    const SceneGraphConfig& scene_graph_config,
    RenderResults* render_results,
    const RenderedSceneDescriptor& frame_id)
{
    LOG_FUNCTION("StandardCameraLogic::render");
    if ((lx.flength() == 0) || (ly.flength() == 0)) {
        THROW_OR_ABORT("StandardCameraLogic::render received zero width or height");
    }
    float aspect_ratio = lx.flength() / ly.flength();

    scene_.delete_node_mutex().notify_reading();
    if (any(frame_id.external_render_pass.pass & ExternalRenderPassType::LIGHTMAP_ANY_MASK)) {
        if (frame_id.external_render_pass.nonstandard_camera_node == nullptr) {
            THROW_OR_ABORT("Lighting pass without camera node");
        }
        camera_node_ = frame_id.external_render_pass.nonstandard_camera_node;
    } else if (frame_id.external_render_pass.pass == ExternalRenderPassType::DIRTMAP) {
        camera_node_ = scene_.get_node(cameras_.dirtmap_node_name(), DP_LOC).ptr();
    } else if (any(frame_id.external_render_pass.pass & ExternalRenderPassType::IMPOSTER_OR_ZOOM_NODE)) {
        if (frame_id.external_render_pass.nonstandard_camera_node == nullptr) {
            THROW_OR_ABORT("Imposter or singular node render pass without camera node");
        }
        camera_node_ = frame_id.external_render_pass.nonstandard_camera_node;
    } else if (frame_id.external_render_pass.pass == ExternalRenderPassType::STANDARD) {
        camera_node_ = scene_.get_node(cameras_.camera_node_name(), DP_LOC).ptr();
    } else {
        THROW_OR_ABORT(
            "StandardCameraLogic::render: unknown render pass: \"" +
            external_render_pass_type_to_string(frame_id.external_render_pass.pass) +
            '"');
    }
    auto camera_object = camera_node_->get_camera().copy();
    camera_object->set_aspect_ratio(aspect_ratio);
    vp_ = dot2d(
        camera_object->projection_matrix().casted<ScenePos>(),
        camera_node_->absolute_view_matrix(frame_id.external_render_pass.time).affine());
    iv_ = camera_node_->absolute_model_matrix(frame_id.external_render_pass.time);
}

float StandardCameraLogic::near_plane() const {
    return scene_.get_node(cameras_.camera_node_name(), DP_LOC)->get_camera().get_near_plane();
}

float StandardCameraLogic::far_plane() const {
    return scene_.get_node(cameras_.camera_node_name(), DP_LOC)->get_camera().get_far_plane();
}

const FixedArray<ScenePos, 4, 4>& StandardCameraLogic::vp() const {
    if (camera_node_ == nullptr) {
        THROW_OR_ABORT("camera node not set in StandardCameraLogic::vp");
    }
    return vp_;
}

const TransformationMatrix<float, ScenePos, 3>& StandardCameraLogic::iv() const {
    if (camera_node_ == nullptr) {
        THROW_OR_ABORT("camera node not set in StandardCameraLogic::iv");
    }
    return iv_;
}

DanglingRef<const SceneNode> StandardCameraLogic::camera_node() const {
    if (camera_node_ == nullptr) {
        THROW_OR_ABORT("camera node not set in StandardCameraLogic::camera_node");
    }
    return *camera_node_;
}

bool StandardCameraLogic::requires_postprocessing() const {
    return scene_.get_node(cameras_.camera_node_name(), DP_LOC)->get_camera().get_requires_postprocessing();
}

void StandardCameraLogic::print(std::ostream& ostr, size_t depth) const {
    ostr << std::string(depth, ' ') << "StandardCameraLogic\n";
}

void StandardCameraLogic::reset() {
    camera_node_ = nullptr;
}
