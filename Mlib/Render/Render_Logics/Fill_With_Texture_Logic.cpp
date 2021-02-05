#include "Fill_With_Texture_Logic.hpp"
#include <Mlib/Geometry/Material/Texture_Descriptor.hpp>
#include <Mlib/Render/CHK.hpp>
#include <Mlib/Render/Render_Logics/Resource_Update_Cycle.hpp>
#include <Mlib/Render/Rendering_Context.hpp>
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
    const std::string& image_resource_name,
    ResourceUpdateCycle update_cycle)
: rendering_resources_{RenderingContextStack::rendering_resources()},
  image_resource_name_{image_resource_name},
  update_cycle_{update_cycle}
{
    rp_.allocate(vertex_shader_text, fragment_shader_text);
    rp_.texture_location = checked_glGetUniformLocation(rp_.program, "texture1");
}

FillWithTextureLogic::~FillWithTextureLogic()
{}

void FillWithTextureLogic::update_texture_id() {
    if ((rp_.texture_id_ == (GLuint)-1) || (update_cycle_ == ResourceUpdateCycle::ALWAYS)) {
        rp_.texture_id_ = rendering_resources_->get_texture({.color = image_resource_name_, .color_mode = ColorMode::RGBA});
    }
}

void FillWithTextureLogic::render(
    int width,
    int height,
    const RenderConfig& render_config,
    const SceneGraphConfig& scene_graph_config,
    RenderResults* render_results,
    const RenderedSceneDescriptor& frame_id)
{
    update_texture_id();

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
