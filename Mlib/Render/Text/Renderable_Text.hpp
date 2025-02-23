#pragma once
#include <Mlib/Array/Fixed_Array.hpp>
#include <Mlib/Render/Any_Gl.hpp>
#include <Mlib/Render/Instance_Handles/Buffer_Background_Copy.hpp>
#include <Mlib/Render/Instance_Handles/Render_Program.hpp>
#include <Mlib/Render/Instance_Handles/Vertex_Array.hpp>
#include <Mlib/Render/Resource_Managers/Font_Name_And_Height.hpp>
#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>

namespace Mlib {

enum class VerticalTextAlignment;
enum class GenericTextAlignment;
enum class TextInterpolationMode;

struct TextRenderProgram: public RenderProgram {
    GLint color_location;
    GLint texture_location;
    GLint projection_location;
};

struct VData {
    VData(Uninitialized)
        : pos{ uninitialized }
        , uv{ uninitialized }
    {}
    VData(float x, float y, float s, float t)
        : pos{ x, y }
        , uv{ s, t }
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
        VariableAndHash<std::string> charset,
        std::string ttf_filename,
        const FixedArray<float, 3>& color,
        size_t max_nchars = 3000);
    void set_contents(
        float font_height,
        const FixedArray<float, 2>& canvas_size,
        TextInterpolationMode interpolation_mode,
        const std::vector<TextAndPosition>& contents);
    void render() const;
    void render(
        float font_height,
        const FixedArray<float, 2>& position,
        const FixedArray<float, 2>& canvas_size,
        const std::string& text,
        VerticalTextAlignment align,
        TextInterpolationMode interpolation_mode,
        float line_distance);
    void render(
        float font_height,
        const IPixelRegion& evaluated_widget,
        const std::string& text,
        float line_distance,
        TextInterpolationMode interpolation_mode,
        GenericTextAlignment horizontal_alignment,
        GenericTextAlignment vertical_alignment);
    void render(const IPixelRegion& evaluated_widget);
private:
    void ensure_initialized(float font_height) const;
    void deallocate();
    mutable TextRenderProgram rp_;
    mutable BufferBackgroundCopy vertices_;
    mutable VertexArray va_;
    mutable const LoadedFont* loaded_font_;
    mutable const std::unordered_map<char32_t, uint32_t>* loaded_charset_;
    mutable FixedArray<float, 2> canvas_size_;
    mutable TextInterpolationMode interpolation_mode_;

    mutable FontNameAndHeight font_descriptor_;
    FixedArray<float, 3> color_;
    size_t max_nchars_;

    // 2 triangles, 3 vertices, 2 positions, 2 uv
    using Letter = FixedArray<VData, 2, 3>;
    mutable std::vector<Letter> vdata_;
    DeallocationToken deallocation_token_;
};

}
