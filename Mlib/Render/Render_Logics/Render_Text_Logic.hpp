#pragma once
#include <Mlib/Array/Fixed_Array.hpp>
#include <Mlib/Variable_And_Hash.hpp>
#include <memory>
#include <string>

namespace Mlib {

class TextResource;
class ILayoutPixels;

class RenderTextLogic {
public:
    RenderTextLogic(
        VariableAndHash<std::string> charset,
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
    VariableAndHash<std::string> charset_;
    std::string ttf_filename_;
    const FixedArray<float, 3> color_;
    mutable std::unique_ptr<TextResource> renderable_text_;
};

}
