#pragma once
#include <Mlib/Array/Fixed_Array.hpp>
#include <Mlib/Render/Render_Logic.hpp>
#include <memory>

namespace Mlib {

class RenderableText;

class RenderTextLogic: public RenderLogic {
public:
    RenderTextLogic(
        const std::string& ttf_filename,
        const FixedArray<float, 2>& position,
        float font_height_pixels,
        float line_distance_pixels);
    ~RenderTextLogic();

    virtual void initialize(GLFWwindow* window) override;

protected:
    std::unique_ptr<RenderableText> renderable_text_;
    std::string ttf_filename_;
    FixedArray<float, 2> position_;
    float font_height_pixels_;
    float line_distance_pixels_;
    GLFWwindow* window_;
};

}
