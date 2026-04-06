#pragma once
#include <Mlib/Array/Array.hpp>

namespace Mlib {

enum class AlphaChannelMode;

class TileImageCanvas {
public:
    TileImageCanvas(
        size_t width,
        size_t height,
        size_t channels,
        AlphaChannelMode alpha_channel_mode);
    void add(const Array<float>& image, float x, float y, float angle);
    Array<float> canvas() const;
private:
    Array<float> canvas_;
    AlphaChannelMode alpha_channel_mode_;
};

}
