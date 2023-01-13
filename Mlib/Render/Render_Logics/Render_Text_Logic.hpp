#pragma once
#include <memory>
#include <string>

namespace Mlib {

class TextResource;
class ILayoutScalar;

class RenderTextLogic {
public:
    RenderTextLogic(
        std::string ttf_filename,
        const ILayoutScalar& font_height,
        const ILayoutScalar& line_distance);
    ~RenderTextLogic();

protected:
    TextResource& renderable_text() const;
    const ILayoutScalar& line_distance_;
private:
    std::string ttf_filename_;
    const ILayoutScalar& font_height_;
    mutable std::unique_ptr<TextResource> renderable_text_;
};

}
