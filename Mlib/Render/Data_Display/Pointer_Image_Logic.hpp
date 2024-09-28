#pragma once
#include <Mlib/Render/Render_Logics/Fill_With_Texture_Logic.hpp>

namespace Mlib {

template <typename TData, size_t... tshape>
class FixedArray;

class PointerImageLogic: public FillWithTextureLogic {
public:
    explicit PointerImageLogic(
        const std::shared_ptr<ITextureHandle>& texture);
    ~PointerImageLogic();

    void render(
        const FixedArray<float, 2>& canvas_size,
        float angle,
        const FixedArray<float, 2>& center,
        const FixedArray<float, 2, 2, 2>& pointer_corners);
};

}
