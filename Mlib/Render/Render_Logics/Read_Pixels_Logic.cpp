#include "Read_Pixels_Logic.hpp"
#include <Mlib/Geometry/Cameras/Camera.hpp>
#include <Mlib/Images/Revert_Axis.hpp>
#include <Mlib/Images/Vectorial_Pixels.hpp>
#include <Mlib/Layout/Layout_Constraint_Parameters.hpp>
#include <Mlib/Log.hpp>
#include <Mlib/Render/CHK.hpp>
#include <Mlib/Render/Instance_Handles/Frame_Buffer.hpp>
#include <Mlib/Render/Instance_Handles/Render_Guards.hpp>
#include <Mlib/Render/Key_Bindings/Key_Configuration.hpp>
#include <Mlib/Render/Key_Bindings/Lockable_Key_Configurations_Fwd.hpp>
#include <Mlib/Render/Render_Config.hpp>
#include <Mlib/Render/Render_Results.hpp>
#include <Mlib/Render/Render_Setup.hpp>
#include <Mlib/Render/Ui/Button_Press.hpp>
#include <Mlib/Render/Viewport_Guard.hpp>
#include <Mlib/Scene_Graph/Rendered_Scene_Descriptor.hpp>
#include <Mlib/Throw_Or_Abort.hpp>
#include <stb/stb_image_write.h>
#include <stb_cpp/stb_image_load.hpp>

namespace Mlib {
class ReadPixelsLogicKeys {
public:
    explicit ReadPixelsLogicKeys(
        const ButtonStates& button_states,
        const LockableKeyConfigurations& key_configurations)
        : ctrl_p{ button_states, key_configurations, 0, "take_screenshot", "" }
    {}
    ButtonPress ctrl_p;
};
}

using namespace Mlib;

ReadPixelsLogic::ReadPixelsLogic(
    RenderLogic& child_logic,
    const ButtonStates& button_states,
    const LockableKeyConfigurations& key_configurations,
    ReadPixelsRole role)
    : child_logic_{ child_logic }
    , keys_{ std::make_unique<ReadPixelsLogicKeys>(button_states, key_configurations) }
    , role_{ role }
{}

ReadPixelsLogic::~ReadPixelsLogic() {
    on_destroy.clear();
}

std::optional<RenderSetup> ReadPixelsLogic::try_render_setup(
    const LayoutConstraintParameters& lx,
    const LayoutConstraintParameters& ly,
    const RenderedSceneDescriptor& frame_id) const
{
    return child_logic_.try_render_setup(lx, ly, frame_id);
}

bool ReadPixelsLogic::render_optional_setup(
    const LayoutConstraintParameters& lx,
    const LayoutConstraintParameters& ly,
    const RenderConfig& render_config,
    const SceneGraphConfig& scene_graph_config,
    RenderResults* render_results,
    const RenderedSceneDescriptor& frame_id,
    const RenderSetup* setup)
{
    LOG_FUNCTION("ReadPixelsLogic::render");
    if (any(role_ & ReadPixelsRole::INTERMEDIATE) &&
        (render_results != nullptr))
    {
        if (auto oit = render_results->outputs.find(frame_id); oit != render_results->outputs.end())
        {
            auto& o = oit->second;
            if (o.rgb.initialized() || o.depth.initialized()) {
                THROW_OR_ABORT("ReadPixelsLogic::render detected multiple rendering calls");
            }
            ViewportGuard vg{ o.width, o.height };
            auto fbs = std::make_shared<FrameBuffer>(CURRENT_SOURCE_LOCATION);
            // Not setting MSAA
            fbs->configure({
                .width = o.width,
                .height = o.height,
                .target = GL_FRAMEBUFFER,
                .depth_kind = o.depth_kind
            });
            {
                RenderToFrameBufferGuard rfg{ fbs };
                child_logic_.render_auto_setup(
                    LayoutConstraintParameters{
                        .dpi = o.dpi,
                        .min_pixel = 0.f,
                        .end_pixel = (float)o.width},
                    LayoutConstraintParameters{
                        .dpi = o.dpi,
                        .min_pixel = 0.f,
                        .end_pixel = (float)o.height},
                    render_config,
                    scene_graph_config,
                    render_results,
                    frame_id,
                    setup);
            }
            o.rgb = fbs->color_to_array(3);
            if (o.depth_kind == FrameBufferChannelKind::TEXTURE) {
                o.depth = fbs->depth_to_array();
            }
        }
    }
    if (any(role_ & ReadPixelsRole::SCREENSHOT) &&
        keys_->ctrl_p.keys_pressed())
    {
        auto fbs = std::make_shared<FrameBuffer>(CURRENT_SOURCE_LOCATION);
        // Setting MSAA
        fbs->configure({
            .width = lx.ilength(),
            .height = ly.ilength(),
            .target = GL_FRAMEBUFFER,
            .depth_kind = FrameBufferChannelKind::ATTACHMENT,
            .nsamples_msaa = render_config.nsamples_msaa });
        {
            RenderToFrameBufferGuard rfg{ fbs };
            child_logic_.render_auto_setup(
                lx,
                ly,
                render_config,
                scene_graph_config,
                render_results,
                frame_id,
                setup);
        }
        auto im = fbs->color_to_stb_image(3);
        if (!stbi_write_png("screenshot.png", im.width, im.height, im.nrChannels, im.data(), 0)) {
            THROW_OR_ABORT("Could not save screenshot");
        }
    }
    child_logic_.render_auto_setup(
        lx,
        ly,
        render_config,
        scene_graph_config,
        render_results,
        frame_id,
        setup);
    return true;
}

void ReadPixelsLogic::print(std::ostream& ostr, size_t depth) const {
    ostr << std::string(depth, ' ') << "ReadPixelsLogic\n";
    child_logic_.print(ostr, depth + 1);
}
