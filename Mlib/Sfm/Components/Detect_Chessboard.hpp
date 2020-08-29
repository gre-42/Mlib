#pragma once
#include <Mlib/Array/Array.hpp>

namespace Mlib {

class PpmImage;

namespace Sfm{

void detect_chessboard(
    const Array<float>& image,
    const ArrayShape& shape,
    Array<float>& p_x,
    Array<float>& p_y,
    PpmImage& bmp);

}}
