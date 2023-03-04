#pragma once
#include <memory>
#include <string>

namespace Mlib {

class TextResource;
class ILayoutPixels;

class RenderTextLogic {
public:
    RenderTextLogic(
        std::string ttf_filename,
        const ILayoutPixels& font_height,
        const ILayoutPixels& line_distance);
    ~RenderTextLogic();

protected:
    TextResource& renderable_text() const;
    const ILayoutPixels& line_distance_;
    const ILayoutPixels& font_height_;
private:
    std::string ttf_filename_;
    mutable std::unique_ptr<TextResource> renderable_text_;
};

}
