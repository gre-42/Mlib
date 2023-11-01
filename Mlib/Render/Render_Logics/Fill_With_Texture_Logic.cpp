#include "Fill_With_Texture_Logic.hpp"
#include <Mlib/Geometry/Material/Texture_Descriptor.hpp>
#include <Mlib/Layout/Widget.hpp>
#include <Mlib/Log.hpp>
#include <Mlib/Render/CHK.hpp>
#include <Mlib/Render/Deallocate/Render_Deallocator.hpp>
#include <Mlib/Render/Instance_Handles/Render_Guards.hpp>
#include <Mlib/Render/Render_Logics/Resource_Update_Cycle.hpp>
#include <Mlib/Render/Rendering_Context.hpp>
#include <Mlib/Render/Rendering_Resources.hpp>
#include <Mlib/Render/Shader_Version.hpp>
#include <Mlib/Render/Viewport_Guard.hpp>
#include <sstream>

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

std::string fragment_shader_text_layer(size_t layer) {
    std::stringstream sstr;
    sstr << SHADER_VER << FRAGMENT_PRECISION;
    sstr << "in vec2 TexCoords;\n";
    sstr << "out vec4 color;\n";
    sstr << "\n";
    sstr << "uniform sampler2DArray texture1;\n";
    sstr << "\n";
    sstr << "void main()\n";
    sstr << "{\n";
    sstr << "    color = texture(texture1, vec3(TexCoords, " << layer << ")).rgba;\n";
    sstr << "}";
    return sstr.str();
}

FillWithTextureLogic::FillWithTextureLogic(
    RenderingResources& rendering_resources,
    std::string image_resource_name,
    ResourceUpdateCycle update_cycle,
    ColorMode color_mode,
    CullFaceMode cull_face_mode,
    RenderTarget render_target,
    const float* quad_vertices,
    std::optional<size_t> layer)
: GenericPostProcessingLogic{quad_vertices},
  rendering_resources_{rendering_resources},
  image_resource_name_{std::move(image_resource_name)},
  update_cycle_{update_cycle},
  color_mode_{color_mode},
  cull_face_mode_{cull_face_mode},
  render_target_{render_target},
  layer_{layer}
{}

FillWithTextureLogic::~FillWithTextureLogic() = default;

void FillWithTextureLogic::set_image_resource_name(const std::string& image_resource_name) {
    image_resource_name_ = image_resource_name;
    rp_.texture_id_ = (GLuint)-1;
}

void FillWithTextureLogic::update_texture_id() {
    if (!rp_.allocated()) {
        if (layer_.has_value()) {
            rp_.allocate(simple_vertex_shader_text_, fragment_shader_text_layer(layer_.value()).c_str());
        } else {
            rp_.allocate(simple_vertex_shader_text_, fragment_shader_text);
        }
        rp_.texture_location = checked_glGetUniformLocation(rp_.program, "texture1");
    }
    if ((rp_.texture_id_ == (GLuint)-1) || (update_cycle_ == ResourceUpdateCycle::ALWAYS)) {
            rp_.texture_id_ = rendering_resources_.get_texture({
                .color = {.filename = image_resource_name_},
                .color_mode = color_mode_,
                .mipmap_mode = MipmapMode::WITH_MIPMAPS});
    }
}

bool FillWithTextureLogic::texture_is_loaded_and_try_preload() const {
    return rendering_resources_.texture_is_loaded_and_try_preload({.color = {.filename = image_resource_name_}});
}

void FillWithTextureLogic::render()
{
    LOG_FUNCTION("FillWithTextureLogic::render");
    update_texture_id();

    RenderToScreenGuard rsg;
    if (cull_face_mode_ == CullFaceMode::CULL) {
        CHK(glEnable(GL_CULL_FACE));
    }
    bool enable_blend =
        (render_target_ != RenderTarget::NO_BLEND) &&
        (color_mode_ == ColorMode::RGBA);
    if (enable_blend) {
        if (color_mode_ != ColorMode::RGBA) {
            THROW_OR_ABORT("Blend render-target is not disabled but texture is RGBA");
        }
        CHK(glEnable(GL_BLEND));
        if (render_target_ == RenderTarget::CANVAS) {
            CHK(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));
        } else if (render_target_ == RenderTarget::TEXTURE) {
            CHK(glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE_MINUS_SRC_ALPHA));
        } else {
            THROW_OR_ABORT("Unknown render-target");
        }
    }
    CHK(glUseProgram(rp_.program));

    CHK(glUniform1i(rp_.texture_location, 0));
    if (layer_.has_value()) {
        CHK(glBindTexture(GL_TEXTURE_2D_ARRAY, rp_.texture_id_));
        CHK(glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR));
    } else {
        CHK(glBindTexture(GL_TEXTURE_2D, rp_.texture_id_));
        CHK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR));
    }

    CHK(glBindVertexArray(va().vertex_array()));

    CHK(glDrawArrays(GL_TRIANGLES, 0, 6));

    CHK(glBindVertexArray(0));
    if (cull_face_mode_ == CullFaceMode::CULL) {
        CHK(glDisable(GL_CULL_FACE));
    }
    if (enable_blend) {
        CHK(glDisable(GL_BLEND));
        CHK(glBlendFunc(GL_ONE, GL_ZERO));
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
