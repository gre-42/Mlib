#include "Renderable_Text.hpp"
#include <Mlib/Array/Fixed_Array.hpp>
#include <Mlib/Layout/Concrete_Layout_Pixels.hpp>
#include <Mlib/Layout/ILayout_Pixels.hpp>
#include <Mlib/Layout/IWidget.hpp>
#include <Mlib/Layout/Screen_Units.hpp>
#include <Mlib/Memory/Integral_Cast.hpp>
#include <Mlib/Os/Os.hpp>
#include <Mlib/Render/CHK.hpp>
#include <Mlib/Render/Deallocate/Render_Deallocator.hpp>
#include <Mlib/Render/Instance_Handles/Render_Guards.hpp>
#include <Mlib/Render/Instance_Handles/Render_Program.hpp>
#include <Mlib/Render/Rendering_Context.hpp>
#include <Mlib/Render/Resource_Managers/Rendering_Resources.hpp>
#include <Mlib/Render/Shader_Version_3_0.hpp>
#include <Mlib/Render/Text/Align_Text.hpp>
#include <Mlib/Render/Text/Loaded_Font.hpp>
#include <Mlib/Render/Text/Text_And_Position.hpp>
#include <Mlib/Render/Text/Text_Interpolation_Mode.hpp>
#include <Mlib/Render/Viewport_Guard.hpp>
#include <Mlib/Render/linmath.hpp>
#include <stb/stb_truetype.h>

using namespace Mlib;

static const size_t TEXTURE_SIZE = 1024;

static const char* vertex_shader_text =
SHADER_VER
"layout (location = 0) in vec4 vertex; // <vec2 pos, vec2 tex>\n"
"out vec2 TexCoords;\n"
"\n"
"uniform mat4 projection;\n"
"\n"
"void main()\n"
"{\n"
"    gl_Position = projection * vec4(vertex.xy, 0.0, 1.0);\n"
"    TexCoords = vertex.zw;\n"
"}";

static const char* fragment_shader_text =
SHADER_VER FRAGMENT_PRECISION
"in vec2 TexCoords;\n"
"out vec4 color;\n"
"\n"
"uniform vec3 color3;\n"
"uniform sampler2D texture1;\n"
"\n"
"void main()\n"
"{\n"
"    color = vec4(color3, texture(texture1, TexCoords).r);\n"
"}";

TextResource::TextResource(
    std::string ttf_filename,
    const FixedArray<float, 3>& color,
    size_t max_nchars)
    : loaded_font_{ nullptr }
    , canvas_size_{ uninitialized }
    , font_descriptor_{ .ttf_filename = std::move(ttf_filename), .height_pixels = NAN }
    , color_{ color }
    , max_nchars_{ max_nchars }
    , deallocation_token_{ render_deallocator.insert([this]() {deallocate(); }) }
{
    va_.add_array_buffer(vertices_);
}

void TextResource::deallocate() {
    loaded_font_ = nullptr;
    font_descriptor_.hash.reset();
}

void TextResource::ensure_initialized(float font_height) const
{
    if (rp_.allocated()) {
        return;
    }
    font_descriptor_.height_pixels = font_height;
    font_descriptor_.compute_hash();
    vdata_.reserve(max_nchars_);
    if (loaded_font_ != nullptr) {
        THROW_OR_ABORT("loaded_font is not null");
    }
    loaded_font_ = &RenderingContextStack::primary_rendering_resources().get_font_texture(font_descriptor_);
    rp_.allocate(vertex_shader_text, fragment_shader_text);
    rp_.color_location = rp_.get_uniform_location("color3");
    rp_.texture_location = rp_.get_uniform_location("texture1");
    rp_.projection_location = rp_.get_uniform_location("projection");
    {
        // configure VAO/VBO for texture quads
        // -----------------------------------
        va_.initialize();
        vertices_.reserve<Letter>(vdata_.capacity());
        CHK(glEnableVertexAttribArray(0));
        CHK(glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), nullptr));
        CHK(glBindBuffer(GL_ARRAY_BUFFER, 0));
        CHK(glBindVertexArray(0));
    }
}

void TextResource::set_contents(
    float font_height,
    const FixedArray<float, 2>& canvas_size,
    TextInterpolationMode interpolation_mode,
    const std::vector<TextAndPosition>& contents)
{
    ensure_initialized(font_height);
    
    canvas_size_ = canvas_size;
    interpolation_mode_ = interpolation_mode;

    vdata_.clear();
    for (const auto& tp : contents) {
        FixedArray<bool, 2> center = Mlib::isnan(tp.position);
        float x = center(0) ? 0.f : tp.position(0);
        float y = center(1) ? 0.f : tp.position(1) + font_height * float(tp.align == AlignText::TOP);
        size_t line_number = 0;
        for (char cs : tp.text) {
            auto c = (unsigned char)cs;
            if (vdata_.size() == vdata_.capacity()) {
                break;
            }
            if (c >= 32 && c < 128) {
                stbtt_aligned_quad q;
                stbtt_GetBakedQuad(loaded_font_->cdata.data(), TEXTURE_SIZE, TEXTURE_SIZE, c - 32, &x, &y, &q, 1);//1=opengl & d3d10+,0=d3d9
                // update VBO for each character
                vdata_.push_back(Letter{
                    FixedArray<VData, 3>{
                        VData{ q.x0, canvas_size(1) - q.y0 - loaded_font_->bottom_y, q.s0, q.t0 },
                        VData{ q.x0, canvas_size(1) - q.y1 - loaded_font_->bottom_y, q.s0, q.t1 },
                        VData{ q.x1, canvas_size(1) - q.y1 - loaded_font_->bottom_y, q.s1, q.t1 }
                    },
                    FixedArray<VData, 3>{
                        VData{ q.x0, canvas_size(1) - q.y0 - loaded_font_->bottom_y, q.s0, q.t0 },
                        VData{ q.x1, canvas_size(1) - q.y1 - loaded_font_->bottom_y, q.s1, q.t1 },
                        VData{ q.x1, canvas_size(1) - q.y0 - loaded_font_->bottom_y, q.s1, q.t0 }
                    }});
            } else if (c == '\n') {
                x = center(0) ? 0.f : tp.position(0);
                y = center(1) ? 0.f : tp.position(1) + float(++line_number) * tp.line_distance + font_height * float(tp.align == AlignText::TOP);
            }
        }
        for (size_t dim = 0; dim < 2; ++dim) {
            if (center(dim)) {
                float min_v = INFINITY;
                float max_v = -INFINITY;
                for (const auto& t : vdata_) {
                    for (const auto& v : t.flat_iterable()) {
                        min_v = std::min(min_v, v.pos(dim));
                        max_v = std::max(max_v, v.pos(dim));
                    }
                }
                for (auto& t : vdata_) {
                    for (auto& v : t.flat_iterable()) {
                        v.pos(dim) = v.pos(dim) - min_v + canvas_size(dim) / 2.f - (max_v - min_v) / 2;
                    }
                }
            }
        }
        if (interpolation_mode == TextInterpolationMode::NEAREST_NEIGHBOR) {
            for (auto& t : vdata_) {
                for (auto& v : t.flat_iterable()) {
                    v.pos = round(v.pos);
                }
            }
        }
    }
    // update content of VBO memory
    vertices_.bind();
    CHK(glBufferSubData(GL_ARRAY_BUFFER, 0, integral_cast<GLsizeiptr>(sizeof(vdata_[0]) * vdata_.size()), vdata_.data())); // be sure to use glBufferSubData and not glBufferData
    CHK(glBindBuffer(GL_ARRAY_BUFFER, 0));
}

void TextResource::render() const
{
    if (!rp_.allocated()) {
        return;
    }
    notify_rendering(CURRENT_SOURCE_LOCATION);

    // TimeGuard time_guard{"TextResource::render", "TextResource::render"};
    CHK(glEnable(GL_CULL_FACE));
    CHK(glEnable(GL_BLEND));
    CHK(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));
    CHK(glDepthMask(GL_FALSE));

    rp_.use();
    CHK(glUniform3fv(rp_.color_location, 1, color_.flat_begin()));
    CHK(glUniform1i(rp_.texture_location, 0));
    mat4x4 projection;
    mat4x4_ortho(projection, 0, canvas_size_(0), 0, canvas_size_(1), -2, 2);
    CHK(glUniformMatrix4fv(rp_.projection_location, 1, GL_FALSE, (const GLfloat*)projection));
    CHK(glBindTexture(GL_TEXTURE_2D, loaded_font_->texture_handle));
    if (interpolation_mode_ == TextInterpolationMode::NEAREST_NEIGHBOR) {
        CHK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST));
        CHK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST));
    } else {
        CHK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
        CHK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
    }
    va_.bind();

    // render quad
    CHK(glDrawArrays(GL_TRIANGLES, 0, integral_cast<GLsizei>(vdata_.size() * 2 * 3)));
    CHK(glBindVertexArray(0));
    CHK(glDisable(GL_CULL_FACE));
    CHK(glDisable(GL_BLEND));
    CHK(glDepthMask(GL_TRUE));
}

void TextResource::render(
    float font_height,
    const FixedArray<float, 2>& position,
    const FixedArray<float, 2>& canvas_size,
    const std::string& text,
    AlignText align,
    TextInterpolationMode interpolation_mode,
    float line_distance)
{
    set_contents(
        font_height,
        canvas_size,
        interpolation_mode,
        {TextAndPosition{
        .text = text,
        .position = position,
        .align = align,
        .line_distance = line_distance}});
    render();
}

void TextResource::render(
    float font_height,
    const IPixelRegion& evaluated_widget,
    const std::string& text,
    float line_distance,
    TextInterpolationMode interpolation_mode)
{
    auto vg = ViewportGuard::from_widget(evaluated_widget);
    if (vg.has_value()) {
        set_contents(
            font_height,
            {evaluated_widget.width(), evaluated_widget.height()},
            interpolation_mode,
            {TextAndPosition{
            .text = text,
            .position = {0.f, 0.f},
            .align = AlignText::TOP,
            .line_distance = line_distance}});
        render();
    }
}
