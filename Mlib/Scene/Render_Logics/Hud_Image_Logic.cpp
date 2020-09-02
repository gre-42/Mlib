#include "Hud_Image_Logic.hpp"
#include <Mlib/Physics/Containers/Advance_Times.hpp>
#include <Mlib/Render/CHK.hpp>
#include <Mlib/Render/Rendering_Resources.hpp>
#include <Mlib/Scene_Graph/Scene_Node.hpp>
#include <sstream>

using namespace Mlib;

HudImageLogic::HudImageLogic(
    SceneNode& scene_node,
    AdvanceTimes& advance_times,
    RenderingResources& rendering_resources,
    const std::string& image_resource_name,
    const FixedArray<float, 2>& center,
    const FixedArray<float, 2>& size)
: FillWithTextureLogic{rendering_resources, image_resource_name},
  advance_times_{advance_times},
  center_{center},
  size_{size}
{
    scene_node.add_destruction_observer(this);
}

void HudImageLogic::notify_destroyed(void* destroyed_object) {
    advance_times_.schedule_delete_advance_time(this);
}

void HudImageLogic::advance_time(float dt) {
    // do nothing (yet)
}

void HudImageLogic::render(
    int width,
    int height,
    const RenderConfig& render_config,
    const SceneGraphConfig& scene_graph_config,
    RenderResults* render_results,
    const RenderedSceneDescriptor& frame_id)
{
    float aspect_ratio = width / (float)height;

    float vertices[] = {
        // positions                                                 // texCoords
        center_(0) - size_(0) / aspect_ratio, center_(1) + size_(1), 0.0f, 1.0f,
        center_(0) - size_(0) / aspect_ratio, center_(1) - size_(1), 0.0f, 0.0f,
        center_(0) + size_(0) / aspect_ratio, center_(1) - size_(1), 1.0f, 0.0f,

        center_(0) - size_(0) / aspect_ratio, center_(1) + size_(1), 0.0f, 1.0f,
        center_(0) + size_(0) / aspect_ratio, center_(1) - size_(1), 1.0f, 0.0f,
        center_(0) + size_(0) / aspect_ratio, center_(1) + size_(1), 1.0f, 1.0f
    };

    CHK(glEnable(GL_CULL_FACE));
    CHK(glEnable(GL_BLEND));
    CHK(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));

    CHK(glUseProgram(rp_.program));

    CHK(glUniform1i(rp_.texture_location, 0));
    CHK(glBindTexture(GL_TEXTURE_2D, rp_.texture_id_));

    CHK(glBindVertexArray(va_.vertex_array));

    CHK(glBindBuffer(GL_ARRAY_BUFFER, va_.vertex_buffer));
    CHK(glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices));
    CHK(glBindBuffer(GL_ARRAY_BUFFER, 0));

    CHK(glDrawArrays(GL_TRIANGLES, 0, 6));

    CHK(glBindVertexArray(0));
    CHK(glDisable(GL_CULL_FACE));
    CHK(glDisable(GL_BLEND));
}

float HudImageLogic::near_plane() const {
    throw std::runtime_error("HudImageLogic::requires_postprocessing not implemented");
}

float HudImageLogic::far_plane() const {
    throw std::runtime_error("HudImageLogic::requires_postprocessing not implemented");
}

const FixedArray<float, 4, 4>& HudImageLogic::vp() const {
    throw std::runtime_error("HudImageLogic::vp not implemented");
}

bool HudImageLogic::requires_postprocessing() const {
    throw std::runtime_error("HudImageLogic::requires_postprocessing not implemented");
}
