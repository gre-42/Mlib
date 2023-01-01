#include "Read_Pixels_Logic.hpp"
#include <Mlib/Images/Revert_Axis.hpp>
#include <Mlib/Images/Vectorial_Pixels.hpp>
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
: child_logic_{child_logic}
{}

ReadPixelsLogic::~ReadPixelsLogic()
{}

void ReadPixelsLogic::render(
    int width,
    int height,
    const RenderConfig& render_config,
    const SceneGraphConfig& scene_graph_config,
    RenderResults* render_results,
    const RenderedSceneDescriptor& frame_id)
{
    LOG_FUNCTION("ReadPixelsLogic::render");
    if (render_results != nullptr) {
        auto o = render_results->outputs.find(frame_id);
        if (o != render_results->outputs.end()) {
            if (o->second.rgb.initialized() || o->second.depth.initialized()) {
                THROW_OR_ABORT("ReadPixelsLogic::render detected multiple rendering calls");
            }
            ViewportGuard vg{o->second.width, o->second.height};
            FrameBuffer fbs;
            // Not setting MSAA
            fbs.configure({ .width = o->second.width, .height = o->second.height, .depth_kind = o->second.depth_kind });
            RenderToFrameBufferGuard rfg{fbs};
            child_logic_.render(
                o->second.width,
                o->second.height,
                render_config,
                scene_graph_config,
                render_results,
                frame_id);
            {
                VectorialPixels<float, 3> vp{ArrayShape{size_t(o->second.height), size_t(o->second.width)}};
                CHK(glReadPixels(0, 0, o->second.width, o->second.height, GL_RGB, GL_FLOAT, vp->flat_iterable().begin()));
                o->second.rgb = o->second.flip_y ? reverted_axis(vp.to_array(), 1) : vp.to_array();
            }
            if (o->second.depth_kind == FrameBufferChannelKind::TEXTURE) {
                Array<float> sp{ ArrayShape{ size_t(o->second.height), size_t(o->second.width) } };
                CHK(glReadPixels(0, 0, o->second.width, o->second.height, GL_DEPTH_COMPONENT, GL_FLOAT, sp->flat_iterable().begin()));
                o->second.depth = o->second.flip_y ? reverted_axis(sp, 0) : sp;
            }
        }
    }
    child_logic_.render(width, height, render_config, scene_graph_config, render_results, frame_id);
}

float ReadPixelsLogic::near_plane() const {
    return child_logic_.near_plane();
}

float ReadPixelsLogic::far_plane() const {
    return child_logic_.far_plane();
}

const FixedArray<double, 4, 4>& ReadPixelsLogic::vp() const {
    return child_logic_.vp();
}

const TransformationMatrix<float, double, 3>& ReadPixelsLogic::iv() const {
    return child_logic_.iv();
}

bool ReadPixelsLogic::requires_postprocessing() const {
    return child_logic_.requires_postprocessing();
}

void ReadPixelsLogic::print(std::ostream& ostr, size_t depth) const {
    ostr << std::string(depth, ' ') << "ReadPixelsLogic\n";
    child_logic_.print(ostr, depth + 1);
}
