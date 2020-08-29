#include "Dense_Point_Cloud.hpp"
#include <Mlib/Array/Array.hpp>
#include <Mlib/Geometry/Homogeneous.hpp>
#include <Mlib/Images/Bgr565Bitmap.hpp>
#include <Mlib/Images/Coordinates.hpp>
#include <Mlib/Images/Coordinates_Fixed.hpp>
#include <Mlib/Images/Draw_Bmp.hpp>
#include <Mlib/Images/Filters/Filters.hpp>
#include <Mlib/Images/Filters/Median_Filter.hpp>
#include <Mlib/Math/Fixed_Math.hpp>
#include <Mlib/Sfm/Disparity/Epiline_Direction.hpp>
#include <Mlib/Sfm/Disparity/Inverse_Epiline_Direction.hpp>
#include <Mlib/Sfm/Disparity/Traceable_Patch.hpp>
#include <Mlib/Sfm/Rigid_Motion/Initial_Reconstruction2.hpp>
#include <Mlib/Stats/Mean.hpp>

using namespace Mlib;
using namespace Mlib::Sfm;

Array<float> Mlib::Sfm::harris_response_1d(
    const Array<float>& im0,
    const Array<float>& F)
{
    assert(im0.ndim() == 2);
    assert(all(im0.shape() > 0));
    Array<float> res{im0.shape()};
    for(size_t r = 0; r < im0.shape(0); ++r) {
        for(size_t c = 0; c < im0.shape(1); ++c) {
            InverseEpilineDirection ied(r, c, F);
            if (ied.good) {
                ArrayShape id0{a2i(ied.center0 - ied.v0)};
                ArrayShape id1{a2i(ied.center0 + ied.v0)};
                if (all(id0 < im0.shape()) &&
                    all(id1 < im0.shape()))
                {
                    res(r, c) = squared((im0(id1) - im0(id0)) * 0.5f);
                } else {
                    res(r, c) = NAN;
                }
            } else {
                res(r, c) = NAN;
            }
        }
    }
    return res;
}

Array<float> Mlib::Sfm::compute_disparity_gray_single_pixel(
    const Array<float>& im0,
    const Array<float>& im1,
    const Array<float>& F,
    size_t search_length,
    bool l1_normalization,
    Array<float>* dsi,
    float d_multiplier)
{
    assert(im0.ndim() == 2);
    assert(all(im0.shape() == im1.shape()));

    return compute_disparity_rgb_single_pixel(
        im0.reshaped(ArrayShape{1}.concatenated(im0.shape())),
        im1.reshaped(ArrayShape{1}.concatenated(im1.shape())),
        F,
        search_length,
        l1_normalization,
        dsi,
        d_multiplier);
}

Array<float> Mlib::Sfm::compute_disparity_rgb_single_pixel(
    const Array<float>& im0,
    const Array<float>& im1,
    const Array<float>& F,
    size_t search_length,
    bool l1_normalization,
    Array<float>* dsi,
    float d_multiplier)
{
    assert(im0.ndim() == 3);
    assert(all(im0.shape() == im1.shape()));
    assert(all(F.shape() == ArrayShape{3, 3}));

    if (dsi != nullptr) {
        dsi->resize[2 * search_length + 1](im0.shape().erased_first());
        *dsi = NAN;
    }
    Array<float> disparity{im0.shape().erased_first()};
    for(size_t r = 0; r < im0.shape(1); ++r) {
        for(size_t c = 0; c < im0.shape(2); ++c) {
            EpilineDirection ed(r, c, F, l1_normalization);
            float best_diff = INFINITY;
            float best_d = NAN;
            if (ed.good) {
                for(float d = -float(search_length); d <= float(search_length); ++d) {
                    ArrayShape id1{a2i(ed.center1 + ed.v1 * d * d_multiplier)};
                    if (all(id1 < im1.shape().erased_first())) {
                        float cdiff = 0;
                        for(size_t h = 0; h < im0.shape(0); ++h) {
                            cdiff += std::abs(im0(h, r, c) - im1(h, id1(0), id1(1)));
                        }
                        if (cdiff < best_diff) {
                            best_diff = cdiff;
                            best_d = d * d_multiplier;
                        }
                        if (dsi != nullptr) {
                            (*dsi)(size_t(d + search_length), r, c) = cdiff;
                        }
                    }
                }
            }
            // Definition of disparity has to match "reconstruct_disparity" below.
            disparity(r, c) = best_d;
        }
    }
    return disparity;
}

Array<float> Mlib::Sfm::compute_disparity_rgb_patch(
    const Array<float>& im0_rgb,
    const Array<float>& im1_rgb,
    const Array<float>& F,
    size_t search_length,
    float worst_error,
    const ArrayShape& patch_size,
    const ArrayShape& patch_nan_size,
    Array<float>* im0_error,
    Array<bool>* mask,
    Array<float>* prior_disparity,
    Array<float>* prior_strength)
{
    assert(all(im0_rgb.shape() == im1_rgb.shape()));
    assert(all(F.shape() == ArrayShape{3, 3}));
    assert(im0_rgb.ndim() == 3);
    assert(im0_rgb.shape(0) == 3 || im0_rgb.shape(0) == 6);

    Array<float> disparity{im0_rgb.shape().erased_first()};
    if (im0_error != nullptr) {
        im0_error->resize(disparity.shape());
    }
    for(size_t r = 0; r < im0_rgb.shape(1); ++r) {
        for(size_t c = 0; c < im0_rgb.shape(2); ++c) {
            if ((mask != nullptr) && (prior_disparity != nullptr) && (!(*mask)(r, c))) {
                disparity(r, c) = (*prior_disparity)(r, c);
                continue;
            }
            EpilineDirection ed(r, c, F);
            float best_d = NAN;
            if (ed.good) {
                TraceablePatch tp{im0_rgb, ArrayShape{r, c}, patch_size, patch_nan_size};
                if (tp.good_) {
                    float new_pos = tp.new_position_on_line(
                        im1_rgb,
                        a2i(ed.center1),
                        ed.v1,
                        search_length,
                        worst_error,
                        im0_error == nullptr ? nullptr : &(*im0_error)(r, c),
                        prior_disparity == nullptr ? nullptr : &(*prior_disparity)(r, c),
                        prior_strength == nullptr ? nullptr : &(*prior_strength)(r, c));
                    if (!std::isnan(new_pos)) {
                        best_d = new_pos;
                    }
                }
            }
            // Definition of disparity has to match "reconstruct_disparity" below.
            disparity(r, c) = best_d;
        }
    }
    return disparity;
}

Array<float> Mlib::Sfm::iterate_disparity_rgb_patch(
    const Array<float>& im0_gray,
    const Array<float>& im0_rgb,
    const Array<float>& im1_rgb,
    const Array<float>& F,
    size_t search_length,
    float worst_error,
    const ArrayShape& patch_size,
    const ArrayShape& patch_nan_size,
    Array<float>* im0_error,
    Array<float>* initial_disparity,
    bool draw_debug_images)
{
    assert(all(im0_rgb.shape() == im1_rgb.shape()));
    assert(all(F.shape() == ArrayShape{3, 3}));
    assert(im0_rgb.ndim() == 3);
    assert(im0_rgb.shape(0) == 3 || im0_rgb.shape(0) == 6);

    Array<bool> mask = ones<bool>(im0_rgb.shape().erased_first());
    Array<float> prior_disparity = full<float>(mask.shape(), 0.3 / squared(70) / 5);
    Array<float> prior_strength = zeros<float>(mask.shape());

    if (initial_disparity != nullptr) {
        assert(all(initial_disparity->shape() == mask.shape()));
        // prior_disparity = guided_filter(im0_gray, *initial_disparity, ArrayShape{15, 15}, float(1e-3));
        prior_disparity = median_filter_2d(*initial_disparity, 15);
        // for(size_t r = 0; r < im0_rgb.shape(1); ++r) {
        //     for(size_t c = 0; c < im0_rgb.shape(2); ++c) {
        //         mask(r, c) = std::abs(prior_disparity(r, c) - (*initial_disparity)(r, c)) > 10;
        //     }
        // }
    }
    // return mask.casted<float>();

    for(size_t i = 0; i < 10; ++i) {
        if (count_nonzero(mask) == 0) {
            break;
        }
        Array<float> disparity_0 = compute_disparity_rgb_patch(
            im0_rgb,
            im1_rgb,
            F,
            search_length,
            worst_error,
            patch_size,
            patch_nan_size,
            im0_error,
            &mask,
            &prior_disparity,
            &prior_strength);
        // std::cerr << nanmax(abs(disparity_0 - *initial_disparity)) << std::endl;
        // std::cerr << nanmax(abs(disparity_0 - prior_disparity)) << std::endl;
        // draw_nan_masked_grayscale(mask.casted<float>(), 0, 0).save_to_file("mask__--" + std::to_string(i) + ".ppm");
        if (draw_debug_images) {
            draw_nan_masked_grayscale(disparity_0, -50.f, 50.f).save_to_file("disparity_0_i-" + std::to_string(i) + ".ppm");
        }
        // prior_disparity = guided_filter(im0_gray, disparity_0, ArrayShape{15, 15}, float(1e-3));
        // prior_disparity = box_filter_nans_as_zeros_NWE(disparity_0, ArrayShape{15, 15});
        prior_disparity = median_filter_2d(disparity_0, 15);

        // std::cerr << nanmax(abs(disparity_0 - prior_disparity)) << std::endl;
        // std::cerr << "------------" << std::endl;
        // for(size_t r = 0; r < disparity_0.shape(0); ++r) {
        //     for(size_t c = 0; c < disparity_0.shape(1); ++c) {
        //         if (mask(r, c)) {
        //             mask(r, c) = (std::abs(prior_disparity(r, c) - disparity_0(r, c)) > 10);
        //             prior_disparity(r, c) = disparity_0(r, c);
        //             prior_strength(r, c) += 0.3 / squared(70) / 5;
        //         }
        //     }
        // }
        if (draw_debug_images) {
            draw_nan_masked_grayscale(prior_disparity, -50.f, 50.f).save_to_file("prior_disparity-" + std::to_string(i) + ".ppm");
        }
    }
    return prior_disparity;
}

Array<float> Mlib::Sfm::move_along_disparity(
    const Array<float>& disparity,
    const Array<float>& F,
    const Array<float>& img1_rgb,
    const Array<bool>* mask,
    Array<bool>* reverse_mask)
{
    assert(all(disparity.shape() == img1_rgb.shape().erased_first()));
    assert(disparity.ndim() == 2);
    assert(img1_rgb.shape(0) == 3);
    Array<float> result{img1_rgb.shape()};
    if (reverse_mask != nullptr) {
        reverse_mask->resize(mask->shape());
        *reverse_mask = false;
    }
    for(size_t r = 0; r < disparity.shape(0); ++r) {
        for(size_t c = 0; c < disparity.shape(1); ++c) {
            EpilineDirection ed(r, c, F);
            if (!ed.good || std::isnan(disparity(r, c))) {
                result(0, r, c) = NAN;
                result(1, r, c) = NAN;
                result(2, r, c) = NAN;
                continue;
            }
            ArrayShape id1{a2i(ed.center1 + ed.v1 * disparity(r, c))};
            if (all(id1 < disparity.shape())) {
                if (mask != nullptr && reverse_mask != nullptr && (*mask)(r, c)) {
                    (*reverse_mask)(id1(0), id1(1)) = true;
                }
                result(0, r, c) = img1_rgb(0, id1(0), id1(1));
                result(1, r, c) = img1_rgb(1, id1(0), id1(1));
                result(2, r, c) = img1_rgb(2, id1(0), id1(1));
            } else {
                result(0, r, c) = NAN;
                result(1, r, c) = NAN;
                result(2, r, c) = NAN;
            }
        }
    }
    return result;
}

Array<float> Mlib::Sfm::reconstruct_disparity(
    const Array<float>& disparity,
    const Array<float>& F,
    const Array<float>& R,
    const Array<float>& t,
    const Array<float>& intrinsic_matrix,
    Array<float>* condition_number)
{
    Array<float> x{ArrayShape{3}.concatenated(disparity.shape())};
    if (condition_number != nullptr) {
        condition_number->do_resize(disparity.shape());
    }
    for(size_t r = 0; r < disparity.shape(0); ++r) {
        for(size_t c = 0; c < disparity.shape(1); ++c) {
            EpilineDirection ed(r, c, F);
            if (!ed.good || std::isnan(disparity(r, c))) {
                x(0, r, c) = NAN;
                x(1, r, c) = NAN;
                x(2, r, c) = NAN;
                if (condition_number != nullptr) {
                    (*condition_number)(r, c) = NAN;
                }
                continue;
            }
            Array<float> yy0 = ed.center0;
            Array<float> yy1 = ed.center1 + ed.v1 * disparity(r, c);
            //NormalizedProjection np(Array<float>{std::list<Array<float>>{
            //    homogenized_3(yy0).reshaped(ArrayShape{1, 3}),
            //    homogenized_3(yy1).reshaped(ArrayShape{1, 3})}});
            if (false) {
                // Cannot reconstruct points where one of the coordinates
                // equals the direction of movement.
                Array<float> xx3 = initial_reconstruction_x3(
                    R,
                    t,
                    intrinsic_matrix,
                    homogenized_3(yy0).reshaped(ArrayShape{1, 3}),
                    homogenized_3(yy1).reshaped(ArrayShape{1, 3}),
                    //np.normalized_intrinsic_matrix(intrinsic_matrix),
                    //np.yn[0],
                    //np.yn[1],
                    false);
                x(0, r, c) = NAN;
                x(1, r, c) = NAN;
                x(2, r, c) = mean(xx3);  // mean of left/right
            } else {
                // Cannot reconstruct points where both of the coordinates
                // equal the direction of movement.
                Array<float> cn{condition_number == nullptr
                    ? Array<float>()
                    : (*condition_number)[r][c].reshaped(ArrayShape{1})};
                Array<float> xx = initial_reconstruction(
                    R,
                    t,
                    intrinsic_matrix,
                    homogenized_3(yy0).reshaped(ArrayShape{1, 3}),
                    homogenized_3(yy1).reshaped(ArrayShape{1, 3}),
                    false, // points are normalized
                    condition_number == nullptr ? nullptr : &cn);
                    //np.normalized_intrinsic_matrix(intrinsic_matrix),
                    //np.yn[0],
                    //np.yn[1]);
                x(0, r, c) = xx(0, 0);
                x(1, r, c) = xx(0, 1);
                x(2, r, c) = xx(0, 2);
            }
        }
    }
    return x;
}

Array<float> Mlib::Sfm::reconstruct_depth(
    const Array<float>& depth,
    const Array<float>& intrinsic_matrix)
{
    assert(depth.ndim() == 2);
    Array<float> x{ArrayShape{3}.concatenated(depth.shape())};
    FixedArray<float, 3, 3> ipi{inv(intrinsic_matrix)};
    for(size_t r = 0; r < depth.shape(0); ++r) {
        for(size_t c = 0; c < depth.shape(1); ++c) {
            FixedArray<size_t, 2> id{r, c};
            FixedArray<float, 3> y = homogenized_3(i2a(id));
            FixedArray<float, 3> xx = dot(ipi, y);
            xx /= xx(2);
            float cdepth = depth(r, c);
            x(0, r, c) = cdepth * xx(0);
            x(1, r, c) = cdepth * xx(1);
            x(2, r, c) = cdepth * xx(2);
        }
    }
    return x;
}
