#include "Read_Pixels_Logic.hpp"
#include <Mlib/Images/Revert_Axis.hpp>
#include <Mlib/Images/Vectorial_Pixels.hpp>
#include <Mlib/Layout/Layout_Constraint_Parameters.hpp>
#include <Mlib/Log.hpp>
#include <Mlib/Render/CHK.hpp>
#include <Mlib/Render/Instance_Handles/Frame_Buffer.hpp>
#include <Mlib/Render/Instance_Handles/Render_Guards.hpp>
#include <Mlib/Render/Render_Results.hpp>
#include <Mlib/Render/Rendered_Scene_Descriptor.hpp>
#include <Mlib/Render/Viewport_Guard.hpp>
#include <Mlib/Throw_Or_Abort.hpp>

using namespace Mlib;

ReadPixelsLogic::ReadPixelsLogic(RenderLogic& child_logic)
    : child_logic_{ child_logic }
{}

ReadPixelsLogic::~ReadPixelsLogic() {
    on_destroy.clear();
}

void ReadPixelsLogic::render(
    const LayoutConstraintParameters& lx,
    const LayoutConstraintParameters& ly,
    const RenderConfig& render_config,
    const SceneGraphConfig& scene_graph_config,
    RenderResults* render_results,
    const RenderedSceneDescriptor& frame_id)
{
    LOG_FUNCTION("ReadPixelsLogic::render");
    if (render_results != nullptr) {
        if (auto oit = render_results->outputs.find(frame_id); oit != render_results->outputs.end())
        {
            auto& o = oit->second;
            if (o.rgb.initialized() || o.depth.initialized()) {
                THROW_OR_ABORT("ReadPixelsLogic::render detected multiple rendering calls");
            }
            ViewportGuard vg{ o.width, o.height };
            FrameBuffer fbs;
            // Not setting MSAA
            fbs.configure({
                .width = o.width,
                .height = o.height,
                .target = GL_FRAMEBUFFER,
                .depth_kind = o.depth_kind
            });
            {
                RenderToFrameBufferGuard rfg{ fbs };
                child_logic_.render(
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
                    frame_id);
            }
            {
                fbs.bind();
                VectorialPixels<float, 3> vp{ArrayShape{size_t(o.height), size_t(o.width)}};
                CHK(glReadPixels(0, 0, o.width, o.height, GL_RGB, GL_FLOAT, vp->flat_begin()->flat_begin()));
                o.rgb = o.flip_y ? reverted_axis(vp.to_array(), 1) : vp.to_array();
            }
            if (o.depth_kind == FrameBufferChannelKind::TEXTURE) {
                Array<float> sp{ ArrayShape{ size_t(o.height), size_t(o.width) } };
                CHK(glReadPixels(0, 0, o.width, o.height, GL_DEPTH_COMPONENT, GL_FLOAT, sp->flat_begin()));
                o.depth = o.flip_y ? reverted_axis(sp, 0) : sp;
            }
        }
    }
    child_logic_.render(
        lx,
        ly,
        render_config,
        scene_graph_config,
        render_results,
        frame_id);
}

float ReadPixelsLogic::near_plane() const {
    return child_logic_.near_plane();
}

float ReadPixelsLogic::far_plane() const {
    return child_logic_.far_plane();
}

const FixedArray<ScenePos, 4, 4>& ReadPixelsLogic::vp() const {
    return child_logic_.vp();
}

const TransformationMatrix<float, ScenePos, 3>& ReadPixelsLogic::iv() const {
    return child_logic_.iv();
}

bool ReadPixelsLogic::requires_postprocessing() const {
    return child_logic_.requires_postprocessing();
}

void ReadPixelsLogic::reset() {
    child_logic_.reset();
}

void ReadPixelsLogic::print(std::ostream& ostr, size_t depth) const {
    ostr << std::string(depth, ' ') << "ReadPixelsLogic\n";
    child_logic_.print(ostr, depth + 1);
}
