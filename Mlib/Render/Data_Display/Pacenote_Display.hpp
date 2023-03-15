#pragma once
#include <string>
#include <vector>

namespace Mlib {

class RenderLogicGallery;
template <typename TData, size_t... tshape>
class FixedArray;
class TextResource;
class IPixelRegion;

class PacenoteDisplay {
public:
    PacenoteDisplay(
        RenderLogicGallery& gallery,
        TextResource& text,
        std::vector<std::string> exhibits);
    void render(
        float value,
        float font_height,
        const IPixelRegion& evaluated_widget);
private:
    RenderLogicGallery& gallery_;
    TextResource& text_;
    std::vector<std::string> exhibits_;
};

}
