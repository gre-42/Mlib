#include "Hud_Image_Logic.hpp"
#include <Mlib/Physics/Containers/Advance_Times.hpp>
#include <Mlib/Render/CHK.hpp>
#include <Mlib/Render/Rendering_Resources.hpp>
#include <Mlib/Scene_Graph/Elements/Scene_Node.hpp>
#include <sstream>

using namespace Mlib;

HudImageLogic::HudImageLogic(
    SceneNode& scene_node,
    AdvanceTimes& advance_times,
    const std::string& image_resource_name,
    ResourceUpdateCycle update_cycle,
    const FixedArray<float, 2>& center,
    const FixedArray<float, 2>& size)
: FillWithTextureLogic{ image_resource_name, update_cycle },
  advance_times_{ advance_times },
  center_{ center },
  size_{ size },
  is_visible_{ false }
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
    if (!is_visible_) {
        return;
    }
    update_texture_id();

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

    CHK(glBindVertexArray(va().vertex_array));

    CHK(glBindBuffer(GL_ARRAY_BUFFER, va().vertex_buffer));
    CHK(glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices));
    CHK(glBindBuffer(GL_ARRAY_BUFFER, 0));

    CHK(glDrawArrays(GL_TRIANGLES, 0, 6));

    CHK(glBindVertexArray(0));
    CHK(glDisable(GL_CULL_FACE));
    CHK(glDisable(GL_BLEND));
}

void HudImageLogic::notify_rendering(const SceneNode& scene_node, const SceneNode& camera_node) const
{
    is_visible_ = (&scene_node == &camera_node);
}

bool HudImageLogic::requires_render_pass(ExternalRenderPassType render_pass) const {
    return false;
}

bool HudImageLogic::requires_blending_pass(ExternalRenderPassType render_pass) const {
    return true;
}
