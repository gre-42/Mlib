#include "Fill_With_Texture_Logic.hpp"
#include <Mlib/Geometry/Material/Texture_Descriptor.hpp>
#include <Mlib/Layout/Widget.hpp>
#include <Mlib/Log.hpp>
#include <Mlib/Render/CHK.hpp>
#include <Mlib/Render/Deallocate/Render_Deallocator.hpp>
#include <Mlib/Render/Render_Logics/Resource_Update_Cycle.hpp>
#include <Mlib/Render/Rendering_Context.hpp>
#include <Mlib/Render/Rendering_Resources.hpp>
#include <Mlib/Render/Shader_Version.hpp>
#include <Mlib/Render/Viewport_Guard.hpp>

using namespace Mlib;

FillWithTextureRenderProgram::FillWithTextureRenderProgram()
: deallocation_token_{render_deallocator.insert([this](){deallocate();})}
{}

FillWithTextureRenderProgram::~FillWithTextureRenderProgram() = default;

void FillWithTextureRenderProgram::deallocate() {
    texture_id_ = (GLuint)-1;
}

static const char* fragment_shader_text =
SHADER_VER FRAGMENT_PRECISION
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
    std::string image_resource_name,
    ResourceUpdateCycle update_cycle,
    ColorMode color_mode,
    const float* quad_vertices)
: GenericPostProcessingLogic{quad_vertices},
  rendering_resources_{RenderingContextStack::rendering_resources()},
  image_resource_name_{std::move(image_resource_name)},
  update_cycle_{update_cycle},
  color_mode_{color_mode}
{}

FillWithTextureLogic::~FillWithTextureLogic() = default;

void FillWithTextureLogic::set_image_resource_name(const std::string& image_resource_name) {
    image_resource_name_ = image_resource_name;
    rp_.texture_id_ = (GLuint)-1;
}

void FillWithTextureLogic::update_texture_id() {
    if (!rp_.allocated()) {
        rp_.allocate(simple_vertex_shader_text_, fragment_shader_text);
        rp_.texture_location = checked_glGetUniformLocation(rp_.program, "texture1");
    }
    if ((rp_.texture_id_ == (GLuint)-1) || (update_cycle_ == ResourceUpdateCycle::ALWAYS)) {
        rp_.texture_id_ = rendering_resources_->get_texture({
            .color = image_resource_name_,
            .color_mode = color_mode_,
            .mipmap_mode = MipmapMode::WITH_MIPMAPS});
    }
}

void FillWithTextureLogic::render()
{
    LOG_FUNCTION("FillWithTextureLogic::render");
    update_texture_id();

    CHK(glEnable(GL_CULL_FACE));
    if (color_mode_ == ColorMode::RGBA) {
        CHK(glEnable(GL_BLEND));
        CHK(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));
    }

    CHK(glUseProgram(rp_.program));

    CHK(glUniform1i(rp_.texture_location, 0));
    CHK(glBindTexture(GL_TEXTURE_2D, rp_.texture_id_));
    CHK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR));

    CHK(glBindVertexArray(va().vertex_array));

    CHK(glDrawArrays(GL_TRIANGLES, 0, 6));

    CHK(glBindVertexArray(0));
    CHK(glDisable(GL_CULL_FACE));
    if (color_mode_ == ColorMode::RGBA) {
        CHK(glDisable(GL_BLEND));
    }
}

void FillWithTextureLogic::render(
    const LayoutConstraintParameters& lx,
    const LayoutConstraintParameters& ly)
{
    LOG_FUNCTION("FillPixelRegionWithTextureLogic::render");
    auto vg = ViewportGuard::from_widget(PixelRegion{lx, ly});
    if (vg.has_value()) {
        render();
    }
}
