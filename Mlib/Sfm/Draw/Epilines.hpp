#pragma once
#include <Mlib/Array/Array_Forward.hpp>
#include <cstddef>

namespace Mlib {

class Rgb24;
class PpmImage;

namespace Sfm {

void draw_epilines_from_epipole(
    const Array<float>& epipole,
    PpmImage& bmp,
    const Rgb24& color);

void draw_epilines_from_F(
    const Array<float>& F,
    PpmImage& bmp,
    const Rgb24& color,
    size_t spacing = 40);


void draw_inverse_epilines_from_F(
    const Array<float>& F,
    PpmImage& bmp,
    const Rgb24& color,
    size_t spacing = 40);

}}
