#include "Render_Text_Logic.hpp"
#include <Mlib/Render/Text/Renderable_Text.hpp>

using namespace Mlib;

RenderTextLogic::RenderTextLogic(
    const std::string& ttf_filename,
    const FixedArray<float, 2>& position,
    float font_height_pixels,
    float line_distance_pixels)
: ttf_filename_{ttf_filename},
  position_{position},
  font_height_pixels_{font_height_pixels},
  line_distance_pixels_{line_distance_pixels},
  window_{nullptr}
{}

RenderTextLogic::~RenderTextLogic()
{}

void RenderTextLogic::initialize(GLFWwindow* window) {
    if (renderable_text_ != nullptr) {
        throw std::runtime_error("Multiple calls to RenderTextLogic::initialize");
    }
    renderable_text_.reset(new RenderableText{ttf_filename_, font_height_pixels_});
    window_ = window;
}
