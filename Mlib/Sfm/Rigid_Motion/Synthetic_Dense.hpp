#pragma once
#include <Mlib/Images/StbImage3.hpp>

namespace Mlib{ namespace Sfm {

void synthetic_dense(
    const StbImage3& im_bgr,
    StbImage3& im0_bgr,
    StbImage3& im1_bgr);

}}
