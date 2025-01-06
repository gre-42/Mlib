#include "Pacenote_Display.hpp"
#include <Mlib/Layout/IWidget.hpp>
#include <Mlib/Physics/Misc/Pacenote.hpp>
#include <Mlib/Render/Render_Logic_Gallery.hpp>
#include <Mlib/Render/Render_Logics/Clear_Mode.hpp>
#include <Mlib/Render/Render_Logics/Fill_With_Texture_Logic.hpp>
#include <Mlib/Render/Text/Align_Text.hpp>
#include <Mlib/Render/Text/Renderable_Text.hpp>
#include <Mlib/Render/Text/Text_And_Position.hpp>
#include <Mlib/Render/Viewport_Guard.hpp>

using namespace Mlib;

PacenoteDisplay::PacenoteDisplay(
    RenderLogicGallery& gallery,
    TextResource& text,
    std::vector<std::string> pictures_left,
    std::vector<std::string> pictures_right)
: gallery_{gallery},
  text_{text},
  pictures_left_{std::move(pictures_left)},
  pictures_right_{std::move(pictures_right)}
{}

void PacenoteDisplay::render(
    const Pacenote& pacenote,
    float font_height,
    TextInterpolationMode text_interpolation_mode,
    const IPixelRegion& text_evaluated_widget,
    const IPixelRegion& picture_evaluated_widget)
{
    // Render picture
    {
        auto vg = ViewportGuard::from_widget(picture_evaluated_widget);
        if (!vg.has_value()) {
            return;
        }
        const auto& exhibits = (pacenote.direction == PacenoteDirection::LEFT)
            ? pictures_left_
            : pictures_right_;
        if (pacenote.gear < exhibits.size()) {
            gallery_[exhibits[pacenote.gear]]->render(ClearMode::OFF);
        }
    }
    // Render text
    {
        auto vg = ViewportGuard::from_widget(text_evaluated_widget);
        if (!vg.has_value()) {
            return;
        }
        FixedArray<float, 2> canvas_size{text_evaluated_widget.width(), text_evaluated_widget.height()};
        text_.set_contents(
            font_height,
            canvas_size,
            text_interpolation_mode,
            {TextAndPosition{
                .text =
                    std::to_string(pacenote.gear) + " " +
                    std::to_string((int)std::round(pacenote.meters_to_start1 - pacenote.meters_to_start0)),
                    // 
                    // + " (" + std::to_string((int)std::round(value.distance_in_meters)) + ')',
                .position = {0.f, 0.f},
                .align = AlignText::TOP,
                .line_distance = 0.f}});
        text_.render();
    }
}

void PacenoteDisplay::preload() const {
    for (const auto& p : pictures_left_) {
        gallery_[p]->texture_is_loaded_and_try_preload();
    }
    for (const auto& p : pictures_right_) {
        gallery_[p]->texture_is_loaded_and_try_preload();
    }
}
