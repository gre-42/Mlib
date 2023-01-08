#pragma once
#include <Mlib/Array/Fixed_Array.hpp>
#include <memory>

namespace Mlib {

class TextResource;
enum class ScreenUnits;

class RenderTextLogic {
public:
    RenderTextLogic(
        std::string ttf_filename,
        const FixedArray<float, 2>& position,
        float font_height,
        float line_distance,
        ScreenUnits units);
    ~RenderTextLogic();

protected:
    TextResource& renderable_text() const;
    FixedArray<float, 2> position_;
    float line_distance_;
private:
    std::string ttf_filename_;
    float font_height_;
    ScreenUnits units_;
    mutable std::unique_ptr<TextResource> renderable_text_;
};

}
