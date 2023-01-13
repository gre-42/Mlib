#pragma once
#include <memory>
#include <string>

namespace Mlib {

class TextResource;
enum class ScreenUnits;

class RenderTextLogic {
public:
    RenderTextLogic(
        std::string ttf_filename,
        float font_height,
        float line_distance,
        ScreenUnits font_height_units);
    ~RenderTextLogic();

protected:
    TextResource& renderable_text() const;
    float line_distance_;
private:
    std::string ttf_filename_;
    float font_height_;
    ScreenUnits font_height_units_;
    mutable std::unique_ptr<TextResource> renderable_text_;
};

}
