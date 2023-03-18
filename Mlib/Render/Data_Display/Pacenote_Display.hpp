#pragma once
#include <string>
#include <vector>

namespace Mlib {

class RenderLogicGallery;
template <typename TData, size_t... tshape>
class FixedArray;
class TextResource;
class IPixelRegion;
struct ActivePacenote;

class PacenoteDisplay {
public:
    PacenoteDisplay(
        RenderLogicGallery& gallery,
        TextResource& text,
        std::vector<std::string> pictures_left,
        std::vector<std::string> pictures_right);
    void render(
        const ActivePacenote& value,
        float font_height,
        const IPixelRegion& text_evaluated_widget,
        const IPixelRegion& picture_evaluated_widget);
private:
    RenderLogicGallery& gallery_;
    TextResource& text_;
    std::vector<std::string> pictures_left_;
    std::vector<std::string> pictures_right_;
};

}
