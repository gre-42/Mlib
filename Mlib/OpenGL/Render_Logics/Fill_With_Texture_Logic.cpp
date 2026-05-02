
#include "Fill_With_Texture_Logic.hpp"
#include <Mlib/Geometry/Material/Color_Mode.hpp>
#include <Mlib/Geometry/Material/Texture_Descriptor.hpp>
#include <Mlib/Geometry/Texture/ITexture_Handle.hpp>
#include <Mlib/Layout/Widget.hpp>
#include <Mlib/Misc/Log.hpp>
#include <Mlib/OpenGL/CHK.hpp>
#include <Mlib/OpenGL/Clear_Wrapper.hpp>
#include <Mlib/OpenGL/Instance_Handles/Render_Guards.hpp>
#include <Mlib/OpenGL/Render_Logics/Clear_Mode.hpp>
#include <Mlib/OpenGL/Render_Logics/Resource_Update_Cycle.hpp>
#include <Mlib/OpenGL/Rendering_Context.hpp>
#include <Mlib/OpenGL/Resource_Managers/Rendering_Resources.hpp>
#include <Mlib/OpenGL/Shader_Version_3_0.hpp>
#include <Mlib/OpenGL/Viewport_Guard.hpp>
#include <sstream>

using namespace Mlib;

FillWithTextureRenderProgram::FillWithTextureRenderProgram() = default;

FillWithTextureRenderProgram::~FillWithTextureRenderProgram() = default;

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

std::string fragment_shader_text_layer(
    size_t layer,
    std::optional<FixedArray<float, 4>>& uniform_border_color)
{
    std::stringstream sstr;
    sstr << SHADER_VER << FRAGMENT_PRECISION;
    sstr << "in vec2 TexCoords;\n";
    sstr << "out vec4 color;\n";
    sstr << "\n";
    sstr << "uniform sampler2DArray texture1;\n";
    sstr << "\n";
    if (uniform_border_color.has_value()) {
        sstr << "bool is_inside_texture(in vec2 uv) {\n";
        sstr << "    return all(greaterThanEqual(uv, vec2(0.0))) && all(lessThanEqual(uv, vec2(1.0)));\n";
        sstr << "}\n";
    }
    sstr << "void main()\n";
    sstr << "{\n";
    sstr << "    color = texture(texture1, vec3(TexCoords, " << layer << ")).rgba;\n";
    if (uniform_border_color.has_value()) {
        const auto& c = *uniform_border_color;
        sstr << "    if (!is_inside_texture(TexCoords)) {\n";
        sstr << "        color = vec4(" << c(0) << ", " << c(1) << ", " << c(2) << ", " << c(3) << ");\n";
        sstr << "    }";
    }
    sstr << "}";
    return sstr.str();
}

std::string fragment_shader_text_border(std::optional<FixedArray<float, 4>>& uniform_border_color)
{
    std::stringstream sstr;
    sstr << SHADER_VER << FRAGMENT_PRECISION;
    sstr << "in vec2 TexCoords;\n";
    sstr << "out vec4 color;\n";
    sstr << "\n";
    sstr << "uniform sampler2D texture1;\n";
    sstr << "\n";
    if (uniform_border_color.has_value()) {
        sstr << "bool is_inside_texture(in vec2 uv) {\n";
        sstr << "    return all(greaterThanEqual(uv, vec2(0.0))) && all(lessThanEqual(uv, vec2(1.0)));\n";
        sstr << "}\n";
    }
    sstr << "void main()\n";
    sstr << "{\n";
    sstr << "    color = texture(texture1, TexCoords).rgba;\n";
    if (uniform_border_color.has_value()) {
        const auto& c = *uniform_border_color;
        sstr << "    if (!is_inside_texture(TexCoords)) {\n";
        sstr << "        color = vec4(" << c(0) << ", " << c(1) << ", " << c(2) << ", " << c(3) << ");\n";
        sstr << "    }";
    }
    sstr << "}";
    return sstr.str();
}

FillWithTextureLogic::FillWithTextureLogic(
    std::shared_ptr<ITextureHandle> texture,
    CullFaceMode cull_face_mode,
    ContinuousBlendMode blend_mode,
    const float* quad_vertices,
    std::optional<size_t> layer,
    const std::optional<FixedArray<float, 4>>& uniform_border_color)
    : GenericPostProcessingLogic{ quad_vertices }
    , texture_{ std::move(texture) }
    , cull_face_mode_{ cull_face_mode }
    , blend_mode_{ blend_mode }
    , layer_{ layer }
    , uniform_border_color_{ uniform_border_color }
{}

FillWithTextureLogic::~FillWithTextureLogic() = default;

void FillWithTextureLogic::set_image_resource_name(std::shared_ptr<ITextureHandle> texture) {
    texture_ = std::move(texture);
}

void FillWithTextureLogic::ensure_allocated() {
    if (!rp_.allocated()) {
        if (!layer_.has_value() && !uniform_border_color_.has_value()) {
            rp_.allocate(simple_vertex_shader_text_, fragment_shader_text);
        } else if (layer_.has_value()) {
            rp_.allocate(simple_vertex_shader_text_, fragment_shader_text_layer(*layer_, uniform_border_color_).c_str());
        } else {
            rp_.allocate(simple_vertex_shader_text_, fragment_shader_text_border(uniform_border_color_).c_str());
        }
        rp_.texture_location = rp_.get_uniform_location("texture1");
    }
}

bool FillWithTextureLogic::texture_is_loaded_and_try_preload() const {
    return texture_->texture_is_loaded_and_try_preload();
}

void FillWithTextureLogic::render_wo_update_and_bind()
{
    LOG_FUNCTION("FillWithTextureLogic::render");

    ensure_allocated();

    texture_->load_gpu();

    notify_rendering(CURRENT_SOURCE_LOCATION);
    if (cull_face_mode_ == CullFaceMode::CULL) {
        CHK(glEnable(GL_CULL_FACE));
    }
    bool enable_alpha_blend =
        (blend_mode_ == ContinuousBlendMode::ALPHA) &&
        any(texture_->color_mode() & ColorMode::RGBA);
    if (enable_alpha_blend) {
        CHK(glEnable(GL_BLEND));
        CHK(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));
    }
    if (blend_mode_ == ContinuousBlendMode::ADD) {
        CHK(glEnable(GL_BLEND));
        CHK(glBlendFunc(GL_ONE, GL_ONE));
    }
    rp_.use();

    CHK(glUniform1i(rp_.texture_location, 0));
    GLenum target = layer_.has_value()
        ? GL_TEXTURE_2D_ARRAY
        : GL_TEXTURE_2D;
    CHK(glBindTexture(target, texture_->handle<GLuint>()));
    if (texture_->mipmap_mode() == MipmapMode::NO_MIPMAPS) {
        CHK(glTexParameteri(target, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
        CHK(glTexParameteri(target, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
    } else {
        CHK(glTexParameteri(target, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR));
        CHK(glTexParameteri(target, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
    }

    va().bind();

    CHK(glDrawArrays(GL_TRIANGLES, 0, 6));

    CHK(glBindVertexArray(0));
    if (cull_face_mode_ == CullFaceMode::CULL) {
        CHK(glDisable(GL_CULL_FACE));
    }
    if (enable_alpha_blend || (blend_mode_ == ContinuousBlendMode::ADD)) {
        CHK(glDisable(GL_BLEND));
        CHK(glBlendFunc(GL_ONE, GL_ZERO));
    }
}

void FillWithTextureLogic::render(ClearMode clear_mode) {
    switch (clear_mode) {
    case ClearMode::OFF:
        break;
    case ClearMode::COLOR:
        clear_color({ 0.f, 0.f, 0.f, 0.f });
        break;
    default:
        throw std::runtime_error("Unsupported clear mode in FillWithTextureLogic");
    }
    render_wo_update_and_bind();
}

void FillWithTextureLogic::render(
    const LayoutConstraintParameters& lx,
    const LayoutConstraintParameters& ly)
{
    LOG_FUNCTION("FillPixelRegionWithTextureLogic::render");
    auto vg = ViewportGuard::from_widget(PixelRegion{ lx, ly });
    if (vg.has_value()) {
        render_wo_update_and_bind();
    }
}
