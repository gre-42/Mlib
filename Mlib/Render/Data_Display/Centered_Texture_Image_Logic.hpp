#pragma once
#include <Mlib/Render/Render_Logics/Fill_With_Texture_Logic.hpp>

namespace Mlib {

template <typename TData, size_t... tshape>
class FixedArray;
struct ColormapWithModifiers;

class CenteredTextureImageLogic: public FillWithTextureLogic {
public:
    explicit CenteredTextureImageLogic(
        RenderingResources& rendering_resources,
        ColormapWithModifiers image_resource_name);
    ~CenteredTextureImageLogic();

    void render(
        const FixedArray<float, 2>& canvas_size,
        float angle,
        const FixedArray<float, 2, 2, 2>& corners);
};

}
