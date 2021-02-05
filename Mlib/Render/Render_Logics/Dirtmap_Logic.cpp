#include "Dirtmap_Logic.hpp"
#include <Mlib/Geometry/Material/Texture_Descriptor.hpp>
#include <Mlib/Log.hpp>
#include <Mlib/Render/CHK.hpp>
#include <Mlib/Render/Rendered_Scene_Descriptor.hpp>
#include <Mlib/Render/Rendering_Resources.hpp>

using namespace Mlib;

DirtmapLogic::DirtmapLogic(
    RenderLogic& child_logic)
: child_logic_{child_logic},
  rendering_context_{RenderingContextStack::rendering_context()},
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
    if (frame_id.external_render_pass.pass == ExternalRenderPassType::DIRTMAP) {
        throw std::runtime_error("DirtmapLogic received dirtmap rendering");
    }
    if (!generated_) {
        if (filename_ == "") {
            throw std::runtime_error("DirtmapLogic::set_filename not called");
        }
        // Calculate camera position
        {
            RenderingContextGuard rrg{rendering_context_};
            child_logic_.render(0, 0, render_config, scene_graph_config, render_results, {.external_render_pass = {ExternalRenderPassType::DIRTMAP, ""}, .time_id = 0, .light_node_name = ""});
        }
        // Load texture and set alias
        rendering_context_.rendering_resources->add_texture_descriptor(
            "dirtmap",
            TextureDescriptor{
                .color = filename_,
                .color_mode = ColorMode::RGB,
                .mixed = "",
                .overlap_npixels = 0});
        rendering_context_.rendering_resources->set_vp("dirtmap", vp());
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

const TransformationMatrix<float, 3>& DirtmapLogic::iv() const {
    return child_logic_.iv();
}

bool DirtmapLogic::requires_postprocessing() const {
    return child_logic_.requires_postprocessing();
}

void DirtmapLogic::set_filename(const std::string& filename) {
    if (!filename_.empty()) {
        throw std::runtime_error("DirtmapLogic::set_filename called multiple times");
    }
    filename_ = filename;
}
