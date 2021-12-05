#pragma once
#include <glad/gl.h>
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include <Mlib/Array/Fixed_Array.hpp>
#include <Mlib/Render/Instance_Handles/Render_Program.hpp>
#include <Mlib/Render/Instance_Handles/Vertex_Array.hpp>
#include <stb_image/stb_truetype.h>
#include <string>
#include <vector>

namespace Mlib {

struct TextRenderProgram: public RenderProgram {
    GLint projection_location;
};

struct VData {
    VData() = default;
    VData(float x, float y, float s, float t)
    : pos{ x, y },
      uv{ s, t }
    {}
    FixedArray<float, 2> pos;
    FixedArray<float, 2> uv;
};

class TextResource {
public:
    TextResource(
        const std::string& ttf_filename,
        float font_height_pixels = 32.f,
        bool flip_y = true,
        size_t max_nchars = 1000);
    void render(
        const FixedArray<float, 2>& position,
        const std::string& text,
        const FixedArray<int, 2>& screen_size,
        float line_distance_pixels = 32.f,
        bool periodic_position = false) const;
private:
    TextRenderProgram rp_;
    VertexArray va_;
    std::vector<stbtt_bakedchar> cdata_;
    // 2 triangles, 3 vertices, 2 positions, 2 uv
    mutable std::vector<FixedArray<VData, 2, 3>> vdata_;
    GLuint ftex_;
    bool flip_y_;
};

}
