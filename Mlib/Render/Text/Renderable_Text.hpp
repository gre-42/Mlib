#pragma once
#include <Mlib/Array/Fixed_Array.hpp>
#include <Mlib/Render/Any_Gl.hpp>
#include <Mlib/Render/Instance_Handles/Render_Program.hpp>
#include <Mlib/Render/Instance_Handles/Vertex_Array.hpp>
#include <stb_image/stb_truetype.h>
#include <string>
#include <vector>

namespace Mlib {

struct TextRenderProgram: public RenderProgram {
    GLint texture_location;
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

enum class AlignText {
    TOP,
    BOTTOM
};

class IEvaluatedWidget;
enum class ScreenUnits;

class TextResource {
public:
    TextResource(
        std::string ttf_filename,
        float font_height_,
        ScreenUnits font_height_units,
        size_t max_nchars = 1000);
    void render(
        int screen_height_npixels,
        float ydpi,
        const FixedArray<float, 2>& position,
        const FixedArray<float, 2>& size,
        const std::string& text,
        AlignText align,
        float line_distance) const;
    void render(
        int screen_height_npixels,
        float ydpi,
        const IEvaluatedWidget& evaluated_widget,
        const std::string& text,
        float line_distance) const;
private:
    void ensure_initialized(float font_height_pixels) const;
    mutable TextRenderProgram rp_;
    mutable VertexArray va_;
    mutable std::vector<stbtt_bakedchar> cdata_;

    std::string ttf_filename_;
    float font_height_;
    ScreenUnits font_height_units_;
    size_t max_nchars_;

    // 2 triangles, 3 vertices, 2 positions, 2 uv
    mutable std::vector<FixedArray<VData, 2, 3>> vdata_;
    mutable GLuint ftex_;
};

}
