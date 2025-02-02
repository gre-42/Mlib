#pragma once
#include <Mlib/Render/Render_Logics/Fill_With_Texture_Logic.hpp>

namespace Mlib {

template <typename TData, size_t... tshape>
class FixedArray;

class CenteredTextureImageLogic: public FillWithTextureLogic {
public:
    explicit CenteredTextureImageLogic(
        const std::shared_ptr<ITextureHandle>& texture,
        ContinuousBlendMode blend_mode);
    ~CenteredTextureImageLogic();

    void render(
        const FixedArray<float, 2>& canvas_size,
        float angle,
        const FixedArray<float, 2, 2, 2>& corners);
};

}
