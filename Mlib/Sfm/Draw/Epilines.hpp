#pragma once
#include <Mlib/Array/Array_Forward.hpp>
#include <cstddef>

namespace Mlib {

struct Rgb24;
class PpmImage;

namespace Sfm {

void draw_epilines_from_epipole(
    const Array<float>& epipole,
    PpmImage& bmp,
    const Rgb24& color);

void draw_epilines_from_F(
    const FixedArray<float, 3, 3>& F,
    PpmImage& bmp,
    const Rgb24& color,
    size_t spacing = 40);


void draw_inverse_epilines_from_F(
    const FixedArray<float, 3, 3>& F,
    PpmImage& bmp,
    const Rgb24& color,
    size_t spacing = 40);

}}
