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
    TextResource(
        std::string ttf_filename,
        const ILayoutPixels& font_height,
        size_t max_nchars = 1000);
    void set_contents(
        const LayoutConstraintParameters& ly,
        const FixedArray<float, 2>& size,
        const std::vector<TextAndPosition>& contents);
    void render(const FixedArray<float, 2>& size) const;
    void render(const IPixelRegion& evaluated_widget) const;
    void render(
        const LayoutConstraintParameters& ly,
        const FixedArray<float, 2>& position,
        const FixedArray<float, 2>& size,
        const std::string& text,
        AlignText align,
        const ILayoutPixels& line_distance);
    void render(
        const LayoutConstraintParameters& ly,
        const IPixelRegion& evaluated_widget,
        const std::string& text,
        const ILayoutPixels& line_distance);
private:
    void ensure_initialized(float font_height_pixels) const;
    mutable TextRenderProgram rp_;
    mutable VertexArray va_;
    mutable const LoadedFont* loaded_font_;

    std::string ttf_filename_;
    const ILayoutPixels& font_height_;
    size_t max_nchars_;

    // 2 triangles, 3 vertices, 2 positions, 2 uv
    mutable std::vector<FixedArray<VData, 2, 3>> vdata_;
};

}
