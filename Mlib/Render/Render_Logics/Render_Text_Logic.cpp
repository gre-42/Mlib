#include "Render_Text_Logic.hpp"
#include <Mlib/Render/Text/Renderable_Text.hpp>

using namespace Mlib;

RenderTextLogic::RenderTextLogic(
    VariableAndHash<std::string> charset,
    std::string ttf_filename,
    const FixedArray<float, 3>& color,
    const ILayoutPixels& font_height,
    const ILayoutPixels& line_distance)
    : line_distance_{line_distance}
    , font_height_{font_height}
    , charset_{std::move(charset)}
    , ttf_filename_{std::move(ttf_filename)}
    , color_{color}
{}

RenderTextLogic::~RenderTextLogic() = default;

TextResource& RenderTextLogic::renderable_text() const {
    if (renderable_text_ == nullptr) {
        renderable_text_ = std::make_unique<TextResource>(charset_, ttf_filename_, color_);
    }
    return *renderable_text_;
}
