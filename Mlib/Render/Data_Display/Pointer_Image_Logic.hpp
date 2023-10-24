#pragma once
#include <Mlib/Render/Render_Logics/Fill_With_Texture_Logic.hpp>

namespace Mlib {

template <typename TData, size_t... tshape>
class FixedArray;

class PointerImageLogic: public FillWithTextureLogic {
public:
    explicit PointerImageLogic(
        RenderingResources& rendering_resources,
        const std::string& image_resource_name);
    ~PointerImageLogic();

    void render(
        const FixedArray<float, 2>& canvas_size,
        float angle,
        const FixedArray<float, 2>& center,
        const FixedArray<float, 2, 2, 2>& pointer_corners);
};

}
