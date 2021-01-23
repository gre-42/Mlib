#include "Fill_Pixel_Region_With_Texture_Logic.hpp"
#include <Mlib/Render/CHK.hpp>

using namespace Mlib;

FillPixelRegionWithTextureLogic::FillPixelRegionWithTextureLogic(
    const std::string& image_resource_name,
    ResourceUpdateCycle update_cycle,
    const FixedArray<float, 2>& position,
    const FixedArray<float, 2>& size,
    Focus focus_mask,
    bool flip_y)
: FillWithTextureLogic{image_resource_name, update_cycle},
  position_{position},
  size_{size},
  focus_mask_{focus_mask},
  flip_y_{flip_y}
{}

void FillPixelRegionWithTextureLogic::render(
    int width,
    int height,
    const RenderConfig& render_config,
    const SceneGraphConfig& scene_graph_config,
    RenderResults* render_results,
    const RenderedSceneDescriptor& frame_id)
{
    update_texture_id();

    FixedArray<float, 2> size_pix{(float)width, (float)height};
    auto small = 2.f * (position_ / size_pix) - 1.f;
    auto large = 2.f * (position_ + size_) / size_pix - 1.f;
    if (flip_y_) {
        std::swap(small(1), large(1));
        small(1) *= -1;
        large(1) *= -1;
    }
    float vertices[] = {
        // positions        // texCoords
        small(0), large(1), 0.0f, 1.0f,
        small(0), small(1), 0.0f, 0.0f,
        large(0), small(1), 1.0f, 0.0f,

        small(0), large(1), 0.0f, 1.0f,
        large(0), small(1), 1.0f, 0.0f,
        large(0), large(1), 1.0f, 1.0f
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

Focus FillPixelRegionWithTextureLogic::focus_mask() const {
    return focus_mask_;
}
