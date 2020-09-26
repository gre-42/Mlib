#include "Dirtmap_Logic.hpp"
#include <Mlib/Geometry/Texture_Descriptor.hpp>
#include <Mlib/Log.hpp>
#include <Mlib/Render/CHK.hpp>
#include <Mlib/Render/Rendered_Scene_Descriptor.hpp>
#include <Mlib/Render/Rendering_Resources.hpp>

using namespace Mlib;

DirtmapLogic::DirtmapLogic(
    RenderLogic& child_logic,
    RenderingResources& rendering_resources)
: child_logic_{child_logic},
  rendering_resources_{rendering_resources},
  generated_{false}
{}

DirtmapLogic::~DirtmapLogic() {}

void DirtmapLogic::render(
    int width,
    int height,
    const RenderConfig& render_config,
    const SceneGraphConfig& scene_graph_config,
    RenderResults* render_results,
    const RenderedSceneDescriptor& frame_id)
{
    LOG_FUNCTION("DirtmapLogic::render");
    if (frame_id.external_render_pass.pass == ExternalRenderPass::DIRTMAP) {
        throw std::runtime_error("DirtmapLogic received dirtmap rendering");
    }
    if (!generated_) {
        if (filename_ == "") {
            throw std::runtime_error("DirtmapLogic::set_filename not called");
        }
        // Populate camera position
        child_logic_.render(0, 0, render_config, scene_graph_config, render_results, {external_render_pass: {ExternalRenderPass::DIRTMAP, ""}, time_id: 0, light_resource_id: 0});
        // Load texture and set alias
        rendering_resources_.add_texture_descriptor(
            "dirtmap",
            TextureDescriptor{
                color: filename_,
                color_mode: ColorMode::RGB,
                mixed: "",
                overlap_npixels: 0});
        rendering_resources_.set_vp("dirtmap", vp());
        generated_ = true;
    }
}

float DirtmapLogic::near_plane() const {
    return child_logic_.near_plane();
}

float DirtmapLogic::far_plane() const {
    return child_logic_.far_plane();
}

const FixedArray<float, 4, 4>& DirtmapLogic::vp() const {
    return child_logic_.vp();
}

const FixedArray<float, 4, 4>& DirtmapLogic::iv() const {
    return child_logic_.iv();
}

bool DirtmapLogic::requires_postprocessing() const {
    return child_logic_.requires_postprocessing();
}

void DirtmapLogic::set_filename(const std::string& filename) {
    if (filename_ != "") {
        throw std::runtime_error("DirtmapLogic::set_filename called multiple times");
    }
    filename_ = filename;
}
