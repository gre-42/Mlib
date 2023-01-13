#include "Render_Text_Logic.hpp"
#include <Mlib/Render/Text/Renderable_Text.hpp>

using namespace Mlib;

RenderTextLogic::RenderTextLogic(
    std::string ttf_filename,
    const ILayoutScalar& font_height,
    const ILayoutScalar& line_distance)
: line_distance_{line_distance},
  ttf_filename_{std::move(ttf_filename)},
  font_height_{font_height}
{}

RenderTextLogic::~RenderTextLogic() = default;

TextResource& RenderTextLogic::renderable_text() const {
    if (renderable_text_ == nullptr) {
        renderable_text_ = std::make_unique<TextResource>(ttf_filename_, font_height_);
    }
    return *renderable_text_;
}
