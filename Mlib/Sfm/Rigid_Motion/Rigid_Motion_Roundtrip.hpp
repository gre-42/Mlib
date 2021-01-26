#pragma once
#include <Mlib/Geometry/Homogeneous.hpp>
#include <Mlib/Sfm/Rigid_Motion/Rigid_Motion_Roundtrip_Sampler.hpp>

namespace Mlib { namespace Sfm {


template <class TData>
Array<TData> rigid_motion_roundtrip(
    const Array<TData>& im_r_depth,
    const Array<TData>& im_l_depth,
    const Array<TData>& ki,
    const Array<TData>& ke)
{
    assert(im_r_depth.ndim() == 2);
    assert(all(im_r_depth.shape() == im_l_depth.shape()));
    Array<TData> result{im_r_depth.shape()};
    RigidMotionRoundtripSampler rs{ki, ke, im_r_depth, im_l_depth};
    for (size_t r = 0; r < result.shape(0); ++r) {
        for (size_t c = 0; c < result.shape(1); ++c) {
            FixedArray<TData, 2> pos_r_round;
            if (!std::isnan(im_r_depth(r, c))) {
                // std::cerr << "from " << FixedArray<TData, 2>{i2a(r), i2a(c)} << std::endl;
                // std::cerr << "to " << pos_r_round << std::endl;
                if (rs.roundtrip_position(r, c, pos_r_round) && !any(Mlib::isnan(pos_r_round))) {
                    result(r, c) = sum(squared(pos_r_round - FixedArray<TData, 2>{i2fi(r), i2fi(c)}));
                } else {
                    result(r, c) = 0;
                }
            } else {
                result(r, c) = NAN;
            }
        }
    }
    return result;
}

}}
