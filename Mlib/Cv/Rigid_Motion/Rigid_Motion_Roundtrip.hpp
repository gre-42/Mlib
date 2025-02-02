#pragma once
#include <Mlib/Cv/Rigid_Motion/Rigid_Motion_Roundtrip_Sampler.hpp>
#include <Mlib/Geometry/Coordinates/Homogeneous.hpp>

namespace Mlib::Cv {

template <class TData>
Array<TData> rigid_motion_roundtrip(
    const Array<TData>& im_r_depth,
    const Array<TData>& im_l_depth,
    const TransformationMatrix<float, float, 2>& ki_r,
    const TransformationMatrix<float, float, 2>& ki_l,
    const TransformationMatrix<float, float, 3>& ke)
{
    assert(im_r_depth.ndim() == 2);
    assert(all(im_r_depth.shape() == im_l_depth.shape()));
    Array<TData> result{im_r_depth.shape()};
    RigidMotionRoundtripSampler rs{ki_r, ki_l, ke, im_r_depth, im_l_depth};
    for (size_t r = 0; r < result.shape(0); ++r) {
        for (size_t c = 0; c < result.shape(1); ++c) {
            FixedArray<TData, 2> pos_r_round = uninitialized;
            if (!std::isnan(im_r_depth(r, c))) {
                // lerr() << "from " << FixedArray<TData, 2>{i2a(r), i2a(c)};
                // lerr() << "to " << pos_r_round;
                if (rs.roundtrip_position(r, c, pos_r_round) && !any(Mlib::isnan(pos_r_round))) {
                    result(r, c) = sum(squared(pos_r_round - FixedArray<TData, 2>{i2fi(r), i2fi(c)}));
                } else {
                    result(r, c) = NAN;
                }
            } else {
                result(r, c) = NAN;
            }
        }
    }
    return result;
}

}
