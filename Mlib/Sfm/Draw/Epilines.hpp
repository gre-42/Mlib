#pragma once
#include <Mlib/Array/Array_Forward.hpp>
#include <cstddef>

namespace Mlib {

struct Rgb24;
class StbImage;

namespace Sfm {

void draw_epilines_from_epipole(
    const FixedArray<float, 2>& epipole,
    StbImage& bmp,
    const Rgb24& color);

void draw_epilines_from_F(
    const FixedArray<float, 3, 3>& F,
    StbImage& bmp,
    const Rgb24& color,
    size_t spacing = 40);

void draw_inverse_epilines_from_F(
    const FixedArray<float, 3, 3>& F,
    StbImage& bmp,
    const Rgb24& color,
    size_t spacing = 40);

}}
