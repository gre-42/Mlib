#pragma once
#include <Mlib/Images/StbImage.hpp>

namespace Mlib{ namespace Sfm {

void synthetic_dense(
    const StbImage& im_bgr,
    StbImage& im0_bgr,
    StbImage& im1_bgr);

}}
