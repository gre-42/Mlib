#pragma once
#include <Mlib/Array/Fixed_Array.hpp>
#include <Mlib/Render/Any_Gl.hpp>
#include <Mlib/Render/Instance_Handles/Render_Program.hpp>
#include <Mlib/Render/Instance_Handles/Vertex_Array.hpp>
#include <string>
#include <vector>

namespace Mlib {

enum class AlignText;

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

class IPixelRegion;
class ILayoutPixels;
struct LayoutConstraintParameters;
struct TextAndPosition;
struct LoadedFont;

class TextResource {
public:
    explicit TextResource(
        std::string ttf_filename,
        size_t max_nchars = 1000);
    void set_contents(
        float font_height,
        const FixedArray<float, 2>& canvas_size,
        const std::vector<TextAndPosition>& contents);
    void render() const;
    void render(
        float font_height,
        const FixedArray<float, 2>& position,
        const FixedArray<float, 2>& canvas_size,
        const std::string& text,
        AlignText align,
        float line_distance);
    void render(
        float font_height,
        const IPixelRegion& evaluated_widget,
        const std::string& text,
        float line_distance);
private:
    void ensure_initialized(float font_height) const;
    mutable TextRenderProgram rp_;
    mutable VertexArray va_;
    mutable const LoadedFont* loaded_font_;
    mutable FixedArray<float, 2> canvas_size_;

    std::string ttf_filename_;
    size_t max_nchars_;

    // 2 triangles, 3 vertices, 2 positions, 2 uv
    mutable std::vector<FixedArray<VData, 2, 3>> vdata_;
};

}
