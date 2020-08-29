#include "Read_Pixels_Logic.hpp"
#include <Mlib/Images/Revert_Axis.hpp>
#include <Mlib/Images/Vectorial_Pixels.hpp>
#include <Mlib/Log.hpp>
#include <Mlib/Render/CHK.hpp>
#include <Mlib/Render/Instance_Handles/Frame_Buffer.hpp>
#include <Mlib/Render/Render_Results.hpp>
#include <Mlib/Render/Rendered_Scene_Descriptor.hpp>

using namespace Mlib;

ReadPixelsLogic::ReadPixelsLogic(RenderLogic& child_logic)
: child_logic_{child_logic}
{}

void ReadPixelsLogic::initialize(GLFWwindow* window) {
    child_logic_.initialize(window);
}

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
            if (o->second.initialized()) {
                throw std::runtime_error("ReadPixelsLogic::render detected multiple rendering calls");
            }
            FrameBuffer fb;
            fb.configure({width: width, height: height});
            CHK(glBindFramebuffer(GL_FRAMEBUFFER, fb.frame_buffer));
            child_logic_.render(
                width,
                height,
                render_config,
                scene_graph_config,
                render_results,
                frame_id);
            VectorialPixels<float, 3> vp{ArrayShape{size_t(height), size_t(width)}};
            CHK(glReadPixels(0, 0, width, height, GL_RGB, GL_FLOAT, vp->flat_iterable().begin()));
            render_results->outputs[frame_id] = reverted_axis(vp.to_array(), 1);
            CHK(glBindFramebuffer(GL_FRAMEBUFFER, 0));
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

const FixedArray<float, 4, 4>& ReadPixelsLogic::vp() const {
    return child_logic_.vp();
}

bool ReadPixelsLogic::requires_postprocessing() const {
    return child_logic_.requires_postprocessing();
}
