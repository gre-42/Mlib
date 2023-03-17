#pragma once
#include <Mlib/Array/Fixed_Array.hpp>
#include <memory>
#include <string>

namespace Mlib {

class TextResource;
class ILayoutPixels;

class RenderTextLogic {
public:
    RenderTextLogic(
        std::string ttf_filename,
        const FixedArray<float, 3>& color,
        const ILayoutPixels& font_height,
        const ILayoutPixels& line_distance);
    ~RenderTextLogic();

protected:
    TextResource& renderable_text() const;
    const ILayoutPixels& line_distance_;
    const ILayoutPixels& font_height_;
private:
    std::string ttf_filename_;
    const FixedArray<float, 3> color_;
    mutable std::unique_ptr<TextResource> renderable_text_;
};

}
