#include "Tile_Image_Canvas.hpp"
#include <Mlib/Images/Alpha_Channel_Mode.hpp>
#include <Mlib/Images/Transform/Draw_Transformed.hpp>
#include <Mlib/Math/Fixed_Rotation_2D.hpp>
#include <Mlib/Math/Transformation/Transformation_Matrix.hpp>
#include <Mlib/Stats/Min_Max.hpp>

using namespace Mlib;

TileImageCanvas::TileImageCanvas(
    size_t width,
    size_t height,
    size_t channels,
    AlphaChannelMode alpha_channel_mode)
    : canvas_(zeros<float>(ArrayShape{channels, height, width}))
    , alpha_channel_mode_{alpha_channel_mode}
{}

void TileImageCanvas::add(const Array<float>& image, float x, float y, float angle) {
    auto trafo = TransformationMatrix<float, float, 2>{
        fixed_rotation_2d(angle),
        FixedArray<float, 2>{y, x}};
    draw_transformed(image, canvas_, trafo, alpha_channel_mode_);
}

Array<float> TileImageCanvas::canvas() const {
    switch (alpha_channel_mode_) {
    case AlphaChannelMode::OFF:
    case AlphaChannelMode::BLEND:
        return canvas_;
    case AlphaChannelMode::ADD: {
            if ((canvas_.ndim() != 3) || (canvas_.shape(0) == 0)) {
                throw std::runtime_error("Unsupported image dimensionality");
            }
            auto res = canvas_.row_range(0, canvas_.shape(0) - 1).copy();
            for (size_t h = 0; h < res.shape(0); ++h) {
                res[h] /= maximum(canvas_[canvas_.shape(0) - 1], 1e-6f);
            }
            return res;
        }
    }
    throw std::runtime_error("Unsupported blend mode");
}
