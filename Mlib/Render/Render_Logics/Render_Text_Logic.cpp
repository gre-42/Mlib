#include "Render_Text_Logic.hpp"
#include <Mlib/Render/Text/Renderable_Text.hpp>

using namespace Mlib;

RenderTextLogic::RenderTextLogic(
    const std::string& ttf_filename,
    const FixedArray<float, 2>& position,
    float font_height_pixels,
    float line_distance_pixels)
: renderable_text_{new RenderableText{ttf_filename, font_height_pixels}},
  position_{position},
  line_distance_pixels_{line_distance_pixels}
{}

RenderTextLogic::~RenderTextLogic()
{}
