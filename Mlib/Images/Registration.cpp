#include "Registration.hpp"
#include <Mlib/Images/Coordinates.hpp>
#include <Mlib/Images/Features.hpp>
#include <Mlib/Images/Filters/Filters.hpp>
#include <Mlib/Images/Optical_Flow.hpp>

using namespace Mlib;

Array<float> Mlib::patch_registration(
    const Array<float>& image0,
    const Array<float>& image1,
    const ArrayShape& max_window_shape,
    bool preserve_shape)
{
    assert(all(image0.shape() == image1.shape()));
    assert(all(2 * max_window_shape <= image0.shape()));
    Array<float> differences(image0.shape() - 2 * max_window_shape);
    Array<float> flow = nans<float>((ArrayShape{differences.ndim()}.concatenated(differences.shape())));
    Array<float> best_ssd(differences.shape());
    best_ssd = std::numeric_limits<float>::infinity();
    max_window_shape.foreach([&](const ArrayShape& window_shapeU) {
        for (int sign = -1; sign <= 1; sign+=2) {
            if (sign == 1 && all(window_shapeU == 0)) {
                continue;
            }
            //std::cerr << sign << "*" << window_shapeU << std::endl;
            ArrayShape window_shape = sign * window_shapeU;
            differences.shape().foreach([&](const ArrayShape& index) {
                //std::cerr << "w " << window_shape << std::endl;
                differences(index) = squared(
                    image0(index + max_window_shape) -
                    image1(index + max_window_shape + window_shape));
            });
            //std::cerr << "diff " << std::endl << differences << std::endl;
            Array<float> ssd = box_filter(differences, max_window_shape, NAN);
            //std::cerr << "ssd " << std::endl << ssd << std::endl;
            differences.shape().foreach([&](const ArrayShape& index) {
                //std::cerr << "c " << ssd(index) << " " << best_ssd(index) << std::endl;
                if (ssd(index) < best_ssd(index)) {
                    best_ssd(index) = ssd(index);
                    for (size_t j = 0; j < differences.ndim(); j++) {
                        flow[j](index) = static_cast<int>(window_shape(j));
                    }
                }
            });
        }
        //std::cerr << "flow " << flow << std::endl;
    });
    if (preserve_shape) {
        Array<float> res = nans<float>(ArrayShape{differences.ndim()}.concatenated(image0.shape()));
        for (size_t j = 0; j < differences.ndim(); j++) {
            flow.shape().erased_first().foreach([&](const ArrayShape& index) {
                res[j](index + max_window_shape) = flow[j](index);
            });
        }
        return res;
    } else {
        return flow;
    }
}

void Mlib::flow_registration(
    const Array<float>& moving,
    const Array<float>& fixed,
    Array<float>& displacement,
    size_t window_size,
    size_t box_size,
    size_t max_displacement,
    size_t niterations)
{
    assert(all(moving.shape() == fixed.shape()));
    displacement = zeros<float>(ArrayShape{fixed.ndim()}.concatenated(fixed.shape()));
    ArrayShape window_shape(fixed.shape());
    window_shape = window_size;
    ArrayShape box_shape(fixed.shape());
    box_shape = box_size;
    Array<bool> mask = ones<bool>(fixed.shape());
    for (size_t i = 0; i < niterations; ++i) {
        Array<float> registered_moving = apply_displacement(moving, displacement);
        // std::cerr << "r\n" << registered_moving << std::endl;
        Array<float> flow;
        optical_flow(registered_moving, fixed, nullptr, window_shape, max_displacement, flow, mask);
        for (size_t d = 0; d < flow.shape(0); d++) {
            flow[d] = box_filter_nans_as_zeros_NWE(flow[d], box_shape);
        }
        // std::cerr << "flow\n" << flow << std::endl;
        displacement += flow;
    }
}

Array<float> Mlib::apply_displacement(
    const Array<float>& moving,
    const Array<float>& displacement)
{
    assert(moving.ndim() == 2);
    assert(all(moving.shape() == displacement.shape().erased_first()));
    assert(displacement.shape(0) == moving.ndim());
    Array<float> result(moving.shape());
    result.shape().foreach([&](const ArrayShape& index) {
        ArrayShape moving_index{
            index(0) + size_t(displacement[id1](index) + 0.5f),
            index(1) + size_t(displacement[id0](index) + 0.5f)};
        // std::cerr << "id " << index << " new " << moving_index << " dis " << displacement[id0](index) << std::endl;
        if (all(moving_index < moving.shape())) {
            result(index) = moving(index);
        } else {
            result(index) = std::numeric_limits<float>::quiet_NaN();
        }
    });
    return result;
}
