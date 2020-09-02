#pragma once
#include <Mlib/Array/Fixed_Array.hpp>
#include <memory>

namespace Mlib {

class RenderableText;

class RenderTextLogic {
public:
    RenderTextLogic(
        const std::string& ttf_filename,
        const FixedArray<float, 2>& position,
        float font_height_pixels,
        float line_distance_pixels);
    ~RenderTextLogic();

protected:
    std::unique_ptr<RenderableText> renderable_text_;
    FixedArray<float, 2> position_;
    float line_distance_pixels_;
};

}
