#include "Render_Text_Logic.hpp"
#include <Mlib/Render/Text/Renderable_Text.hpp>

using namespace Mlib;

RenderTextLogic::RenderTextLogic(
    const std::string& ttf_filename,
    const FixedArray<float, 2>& position,
    float font_height_pixels,
    float line_distance_pixels)
: position_{position},
  line_distance_pixels_{line_distance_pixels},
  ttf_filename_{ttf_filename},
  font_height_pixels_{font_height_pixels}
{}

RenderTextLogic::~RenderTextLogic()
{}

TextResource& RenderTextLogic::renderable_text() const {
    if (renderable_text_ == nullptr) {
        renderable_text_.reset(new TextResource{ttf_filename_, font_height_pixels_});
    }
    return *renderable_text_;
}
