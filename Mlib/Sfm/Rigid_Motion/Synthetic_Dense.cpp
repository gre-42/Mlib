#include "Synthetic_Dense.hpp"

using namespace Mlib;
using namespace Mlib::Sfm;

void Mlib::Sfm::synthetic_dense(
    const PpmImage& im_bgr,
    PpmImage& im0_bgr,
    PpmImage& im1_bgr)
{
    size_t shift = 20;
    im0_bgr.resize(ArrayShape{im_bgr.shape() - ArrayShape{0, shift}});
    im1_bgr.resize(ArrayShape{im_bgr.shape() - ArrayShape{0, shift}});
    assert_true(im_bgr.shape(1) >= shift);
    for(size_t r = 0; r < im_bgr.shape(0); ++r) {
        for(size_t c = 0; c < im_bgr.shape(1) - shift; ++c) {
            im0_bgr(r, c) = im_bgr(r, c);
            im1_bgr(r, c) = im_bgr(r, c + shift);
        }
    }
}
