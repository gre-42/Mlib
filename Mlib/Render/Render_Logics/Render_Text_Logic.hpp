#pragma once
#include <Mlib/Array/Fixed_Array.hpp>
#include <memory>

namespace Mlib {

class TextResource;

class RenderTextLogic {
public:
    RenderTextLogic(
        const std::string& ttf_filename,
        const FixedArray<float, 2>& position,
        float font_height_pixels,
        float line_distance_pixels);
    ~RenderTextLogic();

protected:
    TextResource& renderable_text() const;
    FixedArray<float, 2> position_;
    float line_distance_pixels_;
private:
    std::string ttf_filename_;
    float font_height_pixels_;
    mutable std::unique_ptr<TextResource> renderable_text_;
};

}
