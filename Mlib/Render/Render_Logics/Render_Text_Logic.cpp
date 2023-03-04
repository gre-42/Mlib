#include "Render_Text_Logic.hpp"
#include <Mlib/Render/Text/Renderable_Text.hpp>

using namespace Mlib;

RenderTextLogic::RenderTextLogic(
    std::string ttf_filename,
    const ILayoutPixels& font_height,
    const ILayoutPixels& line_distance)
: line_distance_{line_distance},
  font_height_{font_height},
  ttf_filename_{std::move(ttf_filename)}
{}

RenderTextLogic::~RenderTextLogic() = default;

TextResource& RenderTextLogic::renderable_text() const {
    if (renderable_text_ == nullptr) {
        renderable_text_ = std::make_unique<TextResource>(ttf_filename_);
    }
    return *renderable_text_;
}
