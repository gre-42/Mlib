#pragma once
#include <glad/gl.h>
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include <Mlib/Array/Array_Forward.hpp>
#include <Mlib/Render/Instance_Handles/Render_Program.hpp>
#include <Mlib/Render/Instance_Handles/Vertex_Array.hpp>
#include <stb_image/stb_truetype.h>
#include <string>

namespace Mlib {

struct TextRenderProgram: public RenderProgram {
    GLint projection_location;
};

class RenderableText {
public:
    RenderableText(
        const std::string& ttf_filename,
        float font_height_pixels = 32.f,
        bool flip_y = true);
    void render(
        const FixedArray<float, 2>& position,
        const std::string& text,
        int screen_width,
        int screen_height,
        float line_distance_pixels = 32.f,
        bool periodic_position = false) const;
private:
    TextRenderProgram rp_;
    VertexArray va_;
    stbtt_bakedchar cdata_[96]; // ASCII 32..126 is 95 glyphs
    GLuint ftex_;
    bool flip_y_;
};

}
