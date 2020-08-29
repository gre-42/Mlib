#pragma once
#include <Mlib/Images/PpmImage.hpp>

namespace Mlib{ namespace Sfm {

void synthetic_dense(
    const PpmImage& im_bgr,
    PpmImage& im0_bgr,
    PpmImage& im1_bgr);

}}
