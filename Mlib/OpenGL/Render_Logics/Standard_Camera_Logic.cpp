#include "Standard_Camera_Logic.hpp"
#include <Mlib/Geometry/Cameras/Camera.hpp>
#include <Mlib/Layout/Layout_Constraint_Parameters.hpp>
#include <Mlib/Math/Fixed_Math.hpp>
#include <Mlib/Math/Transformation/Bijection.hpp>
#include <Mlib/Misc/Log.hpp>
#include <Mlib/OpenGL/CHK.hpp>
#include <Mlib/OpenGL/Render_Setup.hpp>
#include <Mlib/OpenGL/Selected_Cameras/Selected_Cameras.hpp>
#include <Mlib/Scene_Graph/Containers/Scene.hpp>
#include <Mlib/Scene_Graph/Elements/Scene_Node.hpp>
#include <Mlib/Scene_Graph/Rendered_Scene_Descriptor.hpp>
#include <Mlib/Time/Fps/Set_Fps.hpp>
#include <stdexcept>

using namespace Mlib;

StandardCameraLogic::StandardCameraLogic(
    const Scene& scene,
    const SelectedCameras& cameras)
    : scene_{ scene }
    , cameras_{ cameras }
{}

StandardCameraLogic::~StandardCameraLogic() {
    on_destroy.clear();
}

std::optional<RenderSetup> StandardCameraLogic::try_render_setup(
    const LayoutConstraintParameters& lx,
    const LayoutConstraintParameters& ly,
    const RenderedSceneDescriptor& frame_id) const
{
    LOG_FUNCTION("StandardCameraLogic::render");
    if ((lx.flength() == 0) || (ly.flength() == 0)) {
        throw std::runtime_error("StandardCameraLogic::render received zero width or height");
    }
    float aspect_ratio = lx.flength() / ly.flength();

    RenderSetup setup{
        .vp = uninitialized,
        .iv = uninitialized,
        .camera = nullptr,
        .camera_node = nullptr
    };
    if (any(frame_id.external_render_pass.pass & ExternalRenderPassType::LIGHTMAP_ANY_MASK)) {
        if (frame_id.external_render_pass.nonstandard_camera_node == nullptr) {
            throw std::runtime_error("Lighting pass without camera node");
        }
        setup.camera_node = frame_id.external_render_pass.nonstandard_camera_node;
        setup.camera = setup.camera_node->get_camera(CURRENT_SOURCE_LOCATION)->copy();
    } else if (frame_id.external_render_pass.pass == ExternalRenderPassType::DIRTMAP) {
        setup.camera_node = scene_.get_node(cameras_.dirtmap_node_name(), CURRENT_SOURCE_LOCATION).ptr();
        setup.camera = setup.camera_node->get_camera(CURRENT_SOURCE_LOCATION)->copy();
    } else if (frame_id.external_render_pass.pass == ExternalRenderPassType::BILLBOARD_SCENE) {
        setup.camera_node = frame_id.external_render_pass.nonstandard_camera_node;
        setup.camera = setup.camera_node->get_camera(CURRENT_SOURCE_LOCATION)->copy();
    } else if (any(frame_id.external_render_pass.pass & ExternalRenderPassType::IMPOSTER_OR_ZOOM_NODE)) {
        if (frame_id.external_render_pass.nonstandard_camera_node == nullptr) {
            throw std::runtime_error("Imposter or singular node render pass without camera node");
        }
        setup.camera_node = frame_id.external_render_pass.nonstandard_camera_node;
        setup.camera = setup.camera_node->get_camera(CURRENT_SOURCE_LOCATION)->copy();
    } else if (any(frame_id.external_render_pass.pass & ExternalRenderPassType::STANDARD_MASK)) {
        auto can = cameras_.camera(CURRENT_SOURCE_LOCATION);
        setup.camera_node = can.node.ptr();
        setup.camera = can.camera->copy();
    } else {
        throw std::runtime_error(
            "StandardCameraLogic::render: unknown render pass: \"" +
            external_render_pass_type_to_string(frame_id.external_render_pass.pass) +
            '"');
    }
    setup.camera->set_aspect_ratio(aspect_ratio);
    auto bi = setup.camera_node->absolute_bijection(frame_id.external_render_pass.time);
    setup.vp = dot2d(
        setup.camera->projection_matrix().casted<ScenePos>(),
        bi.view.affine());
    setup.iv = bi.model;
    return setup;
}

void StandardCameraLogic::render_with_setup(
    const LayoutConstraintParameters& lx,
    const LayoutConstraintParameters& ly,
    const RenderConfig& render_config,
    const SceneGraphConfig& scene_graph_config,
    RenderResults* render_results,
    const RenderedSceneDescriptor& frame_id,
    const RenderSetup& setup)
{}

void StandardCameraLogic::print(std::ostream& ostr, size_t depth) const {
    ostr << std::string(depth, ' ') << "StandardCameraLogic";
}
