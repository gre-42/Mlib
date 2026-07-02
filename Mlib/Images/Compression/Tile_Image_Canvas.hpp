#pragma once
#include <Mlib/Array/Array.hpp>
#include <Mlib/Images/Transform/Coefficient_Image_Cache.hpp>
#include <cstddef>

namespace Mlib {

enum class AlphaChannelMode;
template <class T, size_t ncoeffs>
class CoefficientImage;

class TileImageCanvas {
public:
    TileImageCanvas(
        size_t width,
        size_t height,
        size_t channels,
        AlphaChannelMode alpha_channel_mode);
    void add(
        const Array<float>& image,
        float x,
        float y,
        float angle,
        CachedCoefficientImage* coeffs);
    Array<float> canvas() const;
private:
    Array<float> canvas_;
    AlphaChannelMode alpha_channel_mode_;
};

}
