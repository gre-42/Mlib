#pragma once
#include <Mlib/Render/Render_Logics/Fill_With_Texture_Logic.hpp>

namespace Mlib {

template <typename TData, size_t... tshape>
class FixedArray;
enum class ColorMode;

class CenteredTextureImageLogic: public FillWithTextureLogic {
public:
    explicit CenteredTextureImageLogic(
        RenderingResources& rendering_resources,
        const std::string& image_resource_name,
        ColorMode color_mode);
    ~CenteredTextureImageLogic();

    void render(
        const FixedArray<float, 2>& canvas_size,
        float angle,
        const FixedArray<float, 2, 2, 2>& corners);
};

}
