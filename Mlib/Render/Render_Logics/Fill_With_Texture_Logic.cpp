#include "Fill_With_Texture_Logic.hpp"
#include <Mlib/Geometry/Texture_Descriptor.hpp>
#include <Mlib/Render/CHK.hpp>
#include <Mlib/Render/Rendering_Resources.hpp>
#include <sstream>

using namespace Mlib;

static const char* fragment_shader_text =
"#version 330 core\n"
"in vec2 TexCoords;\n"
"out vec4 color;\n"
"\n"
"uniform sampler2D texture1;\n"
"\n"
"void main()\n"
"{\n"
"    color = texture(texture1, TexCoords).rgba;\n"
"}";

FillWithTextureLogic::FillWithTextureLogic(
    RenderingResources& rendering_resources,
    const std::string& image_resource_name)
{
    rp_.generate(vertex_shader_text, fragment_shader_text);
    rp_.texture_location = checked_glGetUniformLocation(rp_.program, "texture1");
    rp_.texture_id_ = rendering_resources.get_texture({color: image_resource_name, color_mode: ColorMode::RGBA});
}

void FillWithTextureLogic::render(
    int width,
    int height,
    const RenderConfig& render_config,
    const SceneGraphConfig& scene_graph_config,
    RenderResults* render_results,
    const RenderedSceneDescriptor& frame_id)
{
    CHK(glEnable(GL_CULL_FACE));
    CHK(glEnable(GL_BLEND));
    CHK(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));

    CHK(glUseProgram(rp_.program));

    CHK(glUniform1i(rp_.texture_location, 0));
    CHK(glBindTexture(GL_TEXTURE_2D, rp_.texture_id_));

    CHK(glBindVertexArray(va_.vertex_array));

    CHK(glDrawArrays(GL_TRIANGLES, 0, 6));

    CHK(glBindVertexArray(0));
    CHK(glDisable(GL_CULL_FACE));
    CHK(glDisable(GL_BLEND));
}

float FillWithTextureLogic::near_plane() const {
    throw std::runtime_error("FillWithTextureLogic::requires_postprocessing not implemented");
}

float FillWithTextureLogic::far_plane() const {
    throw std::runtime_error("FillWithTextureLogic::requires_postprocessing not implemented");
}

const FixedArray<float, 4, 4>& FillWithTextureLogic::vp() const {
    throw std::runtime_error("FillWithTextureLogic::vp not implemented");
}

const TransformationMatrix<float, 3>& FillWithTextureLogic::iv() const {
    throw std::runtime_error("FillWithTextureLogic::iv not implemented");
}

bool FillWithTextureLogic::requires_postprocessing() const {
    throw std::runtime_error("FillWithTextureLogic::requires_postprocessing not implemented");
}
