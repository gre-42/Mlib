#include "Renderable_Text.hpp"
#include <Mlib/Array/Fixed_Array.hpp>
#include <Mlib/Render/CHK.hpp>
#include <Mlib/Render/Instance_Handles/Render_Program.hpp>
#include <Mlib/Render/Shader_Version.hpp>
#include <Mlib/Render/Viewport_Guard.hpp>
#include <Mlib/Render/linmath.hpp>
#include <iostream>
#include <memory>

using namespace Mlib;

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
"uniform sampler2D texture1;\n"
"\n"
"void main()\n"
"{\n"
"    color = vec4(1.0, 1.0, 1.0, texture(texture1, TexCoords).r);\n"
"}";

TextResource::TextResource(
    const std::string& ttf_filename,
    float font_height_pixels,
    size_t max_nchars)
: cdata_(96),  // ASCII 32..126 is 95 glyphs
  ttf_filename_{ttf_filename},
  font_height_pixels_{font_height_pixels},
  max_nchars_{max_nchars}
{}

void TextResource::ensure_initialized() const {
    if (rp_.allocated()) {
        return;
    }
    vdata_.reserve(max_nchars_);
    {
        std::unique_ptr<FILE, decltype(&fclose)> f{fopen(ttf_filename_.c_str(), "rb"), fclose};
        if (f == nullptr) {
            throw std::runtime_error("Could not open font file \"" + ttf_filename_ + '"');
        }
        std::vector<unsigned char> temp_bitmap(512 * 512);
        {
            {
                std::vector<unsigned char> ttf_buffer(1 << 20);
                size_t nread = fread(ttf_buffer.data(), 1, 1 << 20, f.get());
                if (nread == 0) {
                    throw std::runtime_error("Could not read from font file");
                }
                stbtt_BakeFontBitmap(ttf_buffer.data(), 0, font_height_pixels_, temp_bitmap.data(), 512, 512, 32, 96, cdata_.data()); // no guarantee this fits!
                // can free ttf_buffer at this point
            }
            CHK(glGenTextures(1, &ftex_));
            CHK(glBindTexture(GL_TEXTURE_2D, ftex_));
            CHK(glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, 512, 512, 0, GL_RED, GL_UNSIGNED_BYTE, temp_bitmap.data()));
            // can free temp_bitmap at this point
        }
        CHK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
    }
    rp_.allocate(vertex_shader_text, fragment_shader_text);
    rp_.projection_location = checked_glGetUniformLocation(rp_.program, "projection");
    {
        // configure VAO/VBO for texture quads
        // -----------------------------------
        CHK(glGenVertexArrays(1, &va_.vertex_array));
        CHK(glGenBuffers(1, &va_.vertex_buffer));
        CHK(glBindVertexArray(va_.vertex_array));
        CHK(glBindBuffer(GL_ARRAY_BUFFER, va_.vertex_buffer));
        CHK(glBufferData(GL_ARRAY_BUFFER, sizeof(vdata_[0]) * vdata_.capacity(), nullptr, GL_DYNAMIC_DRAW));
        CHK(glEnableVertexAttribArray(0));
        CHK(glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), 0));
        CHK(glBindBuffer(GL_ARRAY_BUFFER, 0));
        CHK(glBindVertexArray(0));
    }
}

void TextResource::render(
    const FixedArray<float, 2>& position,
    const FixedArray<float, 2>& size,
    const std::string& text,
    AlignText align,
    float line_distance_pixels) const
{
    ensure_initialized();
    // TimeGuard time_guard{"TextResource::render", "TextResource::render"};
    CHK(glEnable(GL_CULL_FACE));
    CHK(glEnable(GL_BLEND));
    CHK(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));

    CHK(glUseProgram(rp_.program));
    mat4x4 projection;
    mat4x4_ortho(projection, 0, size(0), 0, size(1), -2, 2);
    CHK(glUniformMatrix4fv(rp_.projection_location, 1, GL_FALSE, (const GLfloat*)projection));
    CHK(glBindTexture(GL_TEXTURE_2D, ftex_));
    CHK(glBindVertexArray(va_.vertex_array));

    auto center = Mlib::isnan(position);
    float x = center(0) ? 0.f : position(0);
    float y = center(1) ? 0.f : position(1) + font_height_pixels_ * (align == AlignText::TOP);
    size_t line_number = 0;
    vdata_.clear();
    for (unsigned char c : text) {
        if (vdata_.size() == vdata_.capacity()) {
            break;
        }
        if (c >= 32 && c < 128) {
            stbtt_aligned_quad q;
            stbtt_GetBakedQuad(cdata_.data(), 512, 512, c - 32, &x, &y, &q, 1);//1=opengl & d3d10+,0=d3d9
            // update VBO for each character
            vdata_.push_back(FixedArray<VData, 2, 3>{
                VData{ q.x0, size(1) - q.y0, q.s0, q.t0 },
                VData{ q.x0, size(1) - q.y1, q.s0, q.t1 },
                VData{ q.x1, size(1) - q.y1, q.s1, q.t1 },

                VData{ q.x0, size(1) - q.y0, q.s0, q.t0 },
                VData{ q.x1, size(1) - q.y1, q.s1, q.t1 },
                VData{ q.x1, size(1) - q.y0, q.s1, q.t0 }});
        } else if (c == '\n') {
            x = center(0) ? 0.f : position(0);
            y = center(1) ? 0.f : position(1) + (++line_number) * line_distance_pixels + font_height_pixels_ * (align == AlignText::TOP);
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
                    v.pos(dim) = v.pos(dim) - min_v + size(dim) / 2.f - (max_v - min_v) / 2;
                }
            }
        }
    }
    // update content of VBO memory
    CHK(glBindBuffer(GL_ARRAY_BUFFER, va_.vertex_buffer));
    CHK(glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vdata_[0]) * vdata_.size(), vdata_.data())); // be sure to use glBufferSubData and not glBufferData
    CHK(glBindBuffer(GL_ARRAY_BUFFER, 0));
    // render quad
    CHK(glDrawArrays(GL_TRIANGLES, 0, vdata_.size() * 2 * 3));
    CHK(glDisable(GL_CULL_FACE));
    CHK(glDisable(GL_BLEND));
}

void TextResource::render(
    const FixedArray<float, 2>& position,
    const FixedArray<float, 2>& size,
    const FixedArray<int, 2>& screen_size,
    const std::string& text,
    float line_distance_pixels) const
{
    ensure_initialized();
    auto vg = ViewportGuard::periodic(
        position(0),
        position(1),
        size(0),
        size(1),
        screen_size(0),
        screen_size(1));
    if (vg.has_value()) {
        render(
            {0.f, 0.f},
            {vg.value().width, vg.value().height},
            text,
            AlignText::TOP,
            line_distance_pixels);
    }
}
