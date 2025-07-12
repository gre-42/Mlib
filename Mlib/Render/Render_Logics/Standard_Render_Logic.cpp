#include "Standard_Render_Logic.hpp"
#include <Mlib/Geometry/Cameras/Camera.hpp>
#include <Mlib/Log.hpp>
#include <Mlib/Render/CHK.hpp>
#include <Mlib/Render/Clear_Wrapper.hpp>
#include <Mlib/Render/Instance_Handles/Render_Guards.hpp>
#include <Mlib/Render/Render_Config.hpp>
#include <Mlib/Render/Render_Logics/Clear_Mode.hpp>
#include <Mlib/Render/Render_Setup.hpp>
#include <Mlib/Render/Resource_Managers/Rendering_Resources.hpp>
#include <Mlib/Scene_Graph/Containers/Render_Scene_Thread_Guard.hpp>
#include <Mlib/Scene_Graph/Containers/Scene.hpp>
#include <Mlib/Scene_Graph/Delete_Node_Mutex.hpp>
#include <Mlib/Scene_Graph/Rendered_Scene_Descriptor.hpp>
#include <mutex>

using namespace Mlib;

StandardRenderLogic::StandardRenderLogic(
    const Scene& scene,
    RenderLogic& child_logic,
    const FixedArray<float, 3>& background_color,
    ClearMode clear_mode)
    : scene_{ scene }
    , child_logic_{ child_logic }
    , background_color_{ background_color }
    , clear_mode_{ clear_mode }
{}

StandardRenderLogic::~StandardRenderLogic() {
    on_destroy.clear();
}

std::optional<RenderSetup> StandardRenderLogic::try_render_setup(
    const LayoutConstraintParameters& lx,
    const LayoutConstraintParameters& ly,
    const RenderedSceneDescriptor& frame_id) const
{
    return child_logic_.render_setup(lx, ly, frame_id);
}

void StandardRenderLogic::render_with_setup(
    const LayoutConstraintParameters& lx,
    const LayoutConstraintParameters& ly,
    const RenderConfig& render_config,
    const SceneGraphConfig& scene_graph_config,
    RenderResults* render_results,
    const RenderedSceneDescriptor& frame_id,
    const RenderSetup& setup)
{
    LOG_FUNCTION("StandardRenderLogic::render");

    if (any(frame_id.external_render_pass.pass & ExternalRenderPassType::LIGHTMAP_BLOBS_MASK)) {
        clear_color_and_depth({0.f, 0.f, 0.f, 1.f});
    } else if (any(frame_id.external_render_pass.pass & ExternalRenderPassType::LIGHTMAP_ANY_MASK)) {
        clear_color_and_depth({1.f, 1.f, 1.f, 1.f});
    } else if (
        (frame_id.external_render_pass.pass == ExternalRenderPassType::IMPOSTER_NODE) ||
        (frame_id.external_render_pass.pass == ExternalRenderPassType::STANDARD_FOREGROUND))
    {
        clear_color_and_depth({
            background_color_(0),
            background_color_(1),
            background_color_(2),
            0.f});
    } else if (frame_id.external_render_pass.pass == ExternalRenderPassType::ZOOM_NODE) {
        if (all(isnan(background_color_))) {
            clear_depth(ClearBackend::SHADER);
        } else {
            clear_color_and_depth({
                background_color_(0),
                background_color_(1),
                background_color_(2),
                1.f},
                ClearBackend::SHADER);
        }
    } else {
        if (clear_mode_ == ClearMode::COLOR) {
            clear_color({
                background_color_(0),
                background_color_(1),
                background_color_(2),
                1.f});
        } else if (clear_mode_ == ClearMode::DEPTH) {
            clear_depth();
        } else if (clear_mode_ == ClearMode::COLOR_AND_DEPTH) {
            clear_color_and_depth(
                {
                    background_color_(0),
                    background_color_(1),
                    background_color_(2),
                    1.f
                },
                ClearBackend::SHADER);
        } else if (clear_mode_ == ClearMode::OFF) {
            // Do nothing
        } else {
            THROW_OR_ABORT("Unknown clear mode");
        }
    }

    {
        child_logic_.render_with_setup(
            lx,
            ly,
            render_config,
            scene_graph_config,
            render_results,
            frame_id,
            setup);
            
        if (frame_id.external_render_pass.pass == ExternalRenderPassType::STANDARD_BACKGROUND) {
            return;
        }

        RenderConfigGuard rcg{ render_config, frame_id.external_render_pass.pass };
        RenderSceneThreadGuard rstg{ scene_ };

        scene_.render(
            setup.vp,
            setup.iv,
            setup.camera_node,
            render_config,
            scene_graph_config,
            frame_id);
    }

    // if (frame_id.external_render_pass.pass == ExternalRenderPassType::Pass::STANDARD_WO_POSTPROCESSING ||
    //     frame_id.external_render_pass.pass == ExternalRenderPassType::Pass::STANDARD_WITH_POSTPROCESSING)
    // {
    //     static Fps fps;
    //     fps.tick();
    //     static size_t ctr = 0;
    //     if (ctr++ % (60 * 5) == 0) {
    //         std::stringstream sstr;
    //         sstr << "/tmp/scene_"  <<
    //             std::setfill('0') <<
    //             std::setw(5) <<
    //             ctr <<
    //             "_" << 
    //             fps.fps() <<
    //             ".txt";
    //         std::ofstream f{sstr.str()};
    //         f << scene_ << std::endl;
    //         if (f.fail()) {
    //             THROW_OR_ABORT("Could not write to file " + sstr.str());
    //         }
    //     }
    // }
}

void StandardRenderLogic::print(std::ostream& ostr, size_t depth) const {
    ostr << std::string(depth, ' ') << "StandardRenderLogic\n";
    child_logic_.print(ostr, depth + 1);
}

void StandardRenderLogic::set_background_color(const FixedArray<float, 3>& color) {
    std::scoped_lock lock{ mutex_ };
    background_color_ = color;
}
