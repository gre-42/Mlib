#include <Mlib/Arg_Parser.hpp>
#include <Mlib/Cv/Project_Points.hpp>
#include <Mlib/Floating_Point_Exceptions.hpp>
#include <Mlib/Geometry/Coordinates/Homogeneous.hpp>
#include <Mlib/Images/Draw_Bmp.hpp>
#include <Mlib/Images/Filters/Box_Filter.hpp>
#include <Mlib/Images/Filters/Central_Differences.hpp>
#include <Mlib/Images/Filters/Filters.hpp>
#include <Mlib/Images/Filters/Guided_Filter.hpp>
#include <Mlib/Images/Filters/Median_Filter.hpp>
#include <Mlib/Images/StbImage3.hpp>
#include <Mlib/Sfm/Disparity/Dense_Point_Cloud.hpp>
#include <Mlib/Sfm/Draw/Dense_Projector.hpp>
#include <Mlib/Sfm/Draw/Epilines.hpp>
#include <Mlib/Sfm/Rigid_Motion/Fundamental_Matrix.hpp>
#include <Mlib/Sfm/Rigid_Motion/Synthetic_Dense.hpp>
#include <Mlib/Stats/Histogram.hpp>
#include <Mlib/Stats/Neighbor_Db.hpp>
#include <filesystem>

namespace fs = std::filesystem;

using namespace Mlib;
using namespace Mlib::Cv;
using namespace Mlib::Sfm;

void filter_distance_to_camera(Array<float>& x, Array<float>& cond, const Array<float>& im) {
    assert(x.ndim() == 3);
    assert(x.shape(0) == 3);
    Array<float> dist{x.shape().erased_first()};
    for (size_t r = 0; r < x.shape(1); ++r) {
        for (size_t c = 0; c < x.shape(2); ++c) {
            dist(r, c) = std::sqrt(
                squared(x(0, r, c)) +
                squared(x(1, r, c)) +
                squared(x(2, r, c)));
        }
    }
    draw_nan_masked_grayscale(dist, -25.f, 25.f).save_to_file("dist.png");
    Array<float> distf = guided_filter(im, dist, ArrayShape{15, 15}, float(1e-3));
    draw_nan_masked_grayscale(distf, -25.f, 25.f).save_to_file("distf.png");
    draw_nan_masked_grayscale(abs(dist - distf), 0.f, 1.f).save_to_file("dist-distf.png");
    if (false) {
        for (size_t r = 0; r < x.shape(1); ++r) {
            for (size_t c = 0; c < x.shape(2); ++c) {
                for (size_t d = 0; d < x.shape(0); ++d) {
                    x(d, r, c) /= dist(r, c);
                }
            }
        }
    }
    if (true) {
        for (size_t r = 0; r < x.shape(1); ++r) {
            for (size_t c = 0; c < x.shape(2); ++c) {
                if (!std::isnan(dist(r, c)) &&
                    !std::isnan(distf(r, c)) &&
                    std::abs(dist(r, c) - distf(r, c)) < 0.5)
                {
                    for (size_t d = 0; d < x.shape(0); ++d) {
                        x(d, r, c) = x(d, r, c);
                    }
                } else {
                    for (size_t d = 0; d < x.shape(0); ++d) {
                        x(d, r, c) = NAN;
                    }
                    cond(r, c) = NAN;
                }
            }
        }
    }
}

void reproject(
    const TransformationMatrix<float, float, 2>& intrinsic_matrix,
    const Array<float>& x,
    const Array<float>& im0_rgb)
{
    Array<float> reprojected{im0_rgb.shape()};
    for (size_t r = 0; r < reprojected.shape(1); ++r) {
        for (size_t c = 0; c < reprojected.shape(2); ++c) {
            if (std::isnan(x(0, r, c))) {
                reprojected(0, r, c) = NAN;
                reprojected(1, r, c) = NAN;
                reprojected(2, r, c) = NAN;
                continue;
            }
            FixedArray<float, 2> proj = projected_points_1p_1ke(
                FixedArray<float, 3>{x(0, r, c), x(1, r, c), x(2, r, c)},
                intrinsic_matrix,
                TransformationMatrix<float, float, 3>::identity());
            FixedArray<size_t, 2> id{a2i(proj)};
            // lerr() << proj;
            // reprojected(r, c) = sum(squared(proj, i2a(proj;
            // lerr() << id << " | " << ArrayShape{r, c};
            // lerr() << im0_rgb(0, r, c) << " " << im0_rgb(1, r, c) << " " << im0_rgb(2, r, c);
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmaybe-uninitialized"
            if (all(id < FixedArray<size_t, 2>{reprojected.shape().erased_first()})) {
#pragma GCC diagnostic pop
                if (false) {
                    reprojected(0, r, c) = im0_rgb(0, id(0), id(1));
                    reprojected(1, r, c) = im0_rgb(1, id(0), id(1));
                    reprojected(2, r, c) = im0_rgb(2, id(0), id(1));
                } else {
                    reprojected(0, r, c) = std::sqrt(squared(x(0, r, c)) + squared(x(1, r, c)) + squared(x(2, r, c))) / 5;
                    reprojected(1, r, c) = std::sqrt(squared(x(0, r, c)) + squared(x(1, r, c)) + squared(x(2, r, c))) / 5;
                    reprojected(2, r, c) = std::sqrt(squared(x(0, r, c)) + squared(x(1, r, c)) + squared(x(2, r, c))) / 5;
                }
            } else {
                reprojected(0, r, c) = NAN;
                reprojected(1, r, c) = NAN;
                reprojected(2, r, c) = NAN;
            }
        }
    }
    draw_nan_masked_rgb(reprojected, 0, 1).save_to_file("reprojected.png");
}

class ColorDescriptor {
public:
    explicit ColorDescriptor(const Array<float>& im_rgb)
    : im_rgb_(im_rgb) {
        assert(im_rgb.ndim() == 3);
        assert(im_rgb.shape(0) == 3);
    }

    bool can_compute(size_t r, size_t c) const {
        return true;
    }

    Array<float> operator () (size_t r, size_t c) const {
        return Array<float>{
            im_rgb_(0, r, c),
            im_rgb_(1, r, c),
            im_rgb_(2, r, c)};
    }
private:
    const Array<float> im_rgb_;
};


int main(int argc, char **argv) {
    enable_floating_point_exceptions();

    ArgParser parser(
        "Usage: sfm_dense --intrinsic_matrix <intrinsic_matrix.m> --im0 <image0.bgr> --im1 <image1.bgr> --c0 <camera0.m> --c1 <camera1.m>",
        {},
        {"--intrinsic_matrix", "--im0", "--im1", "--c0", "--c1"});

    try {
        auto args = parser.parsed(argc, argv);

        const bool synthetic = false;

        StbImage3 im0_bgr = StbImage3::load_from_file(args.named_value("--im0"));
        StbImage3 im1_bgr = StbImage3::load_from_file(args.named_value("--im1"));
        if (any(im0_bgr.shape() != im1_bgr.shape())) {
            throw std::runtime_error("Image shapes differ");
        }
        if (synthetic) {
            StbImage3 im_bgr{im0_bgr.copy()};
            synthetic_dense(im_bgr, im0_bgr, im1_bgr);
        }
        Array<float> im0_rgb = im0_bgr.to_float_rgb();
        Array<float> im1_rgb = im1_bgr.to_float_rgb();

        Array<float> im0_gray = im0_bgr.to_float_grayscale();
        Array<float> im1_gray = im1_bgr.to_float_grayscale();

        TransformationMatrix<float, float, 2> intrinsic_matrix{ FixedArray<float, 3, 3>{ Array<float>::load_txt_2d(args.named_value("--intrinsic_matrix")) } };

        Array<float> c0 = Array<float>::load_txt_2d(args.named_value("--c0"));
        Array<float> c1 = Array<float>::load_txt_2d(args.named_value("--c1"));

        Array<float> dc = reconstruction_in_reference(c0, c1);
        TransformationMatrix<float, float, 3> ke{ FixedArray<float, 4, 4>{ dc } };
        if (synthetic) {
            ke.t = FixedArray<float, 3>{1.f, 0.f, 0.f };
            ke.R = fixed_identity_array<float, 3>();
        }
        FixedArray<float, 3, 3> F = fundamental_from_camera(intrinsic_matrix, intrinsic_matrix, ke);

        Array<float> disparity_0;
        Array<float> error_0;
        Array<float> condition_number;
        Array<float> x;

        // Motivation: image must be sampled at least with half the pixel-size.
        // => Either upsample and interpolate, or low-pass-filter the image.
        // Array<float> im0_rgb_smooth{im0_rgb.shape() + ArrayShape{3, 0, 0}};
        // Array<float> im1_rgb_smooth{im1_rgb.shape() + ArrayShape{3, 0, 0}};
        // for (size_t d = 0; d < 3; ++d) {
        //     im0_rgb_smooth[d] = box_filter_nan(im0_rgb[d], ArrayShape{5, 5}, NAN);
        //     im1_rgb_smooth[d] = box_filter_nan(im1_rgb[d], ArrayShape{5, 5}, NAN);
        // }
        // for (size_t d = 3; d < 6; ++d) {
        //     im0_rgb_smooth[d] = im0_rgb[d - 3];
        //     im1_rgb_smooth[d] = im1_rgb[d - 3];
        // }
        // draw_nan_masked_rgb(im0_rgb_smooth, 0, 1).save_to_file("im0_rgb_smooth.png");
        // draw_nan_masked_rgb(im1_rgb_smooth, 0, 1).save_to_file("im1_rgb_smooth.png");
        // Array<float> im0_gray_smooth = box_filter_nan(im0_gray, ArrayShape{3, 3}, NAN);

        if (!fs::exists("condition_number.m")) {
            {
                StbImage3 bmp = StbImage3::from_float_rgb(im0_rgb);
                draw_epilines_from_F(F, bmp, Rgb24::green());
                bmp.save_to_file("epilines-0.png");
            }
            {
                StbImage3 bmp = StbImage3::from_float_rgb(im0_rgb);
                draw_inverse_epilines_from_F(F, bmp, Rgb24::green());
                bmp.save_to_file("epilines-i-0.png");
            }
            {
                StbImage3 bmp = StbImage3::from_float_rgb(im1_rgb);
                draw_epilines_from_F(F.T(), bmp, Rgb24::green());
                bmp.save_to_file("epilines-1.png");
            }

            Array<float> im0_gray_masked;
            Array<float> im1_gray_masked;
            im0_gray_masked = im0_gray;
            im1_gray_masked = im1_gray;
            if (false) {
                Array<float> stddev0 = standard_deviation(im0_gray, ArrayShape{5, 5}, NAN);
                Array<float> stddev1 = standard_deviation(im1_gray, ArrayShape{5, 5}, NAN);
                for (size_t r = 0; r < stddev0.shape(0); ++r) {
                    for (size_t c = 0; c < stddev0.shape(1); ++c) {
                        // lerr() << stddev(r, c);
                        if (!(stddev0(r, c) > 0.001)) {
                            for (size_t d = 0; d < 3; ++d) {
                                im0_rgb(d, r, c) = NAN;
                            }
                            im0_gray_masked(r, c) = NAN;
                        }
                        if (!(stddev1(r, c) > 0.001)) {
                            for (size_t d = 0; d < 3; ++d) {
                                im1_rgb(d, r, c) = NAN;
                            }
                            im1_gray_masked(r, c) = NAN;
                        }
                    }
                }
                draw_nan_masked_grayscale(im0_gray_masked, 0, 1).save_to_file("stdm-0.png");
                draw_nan_masked_grayscale(im1_gray_masked, 0, 1).save_to_file("stdm-1.png");
            }
            if (false) {
                Array<float> hr0_1d = harris_response_1d(im0_gray, F);
                Array<float> hr1_1d = harris_response_1d(im1_gray, F.T());
                Array<float> hr0_1d_im = hr0_1d / float(1e-3);
                Array<float> hr1_1d_im = hr1_1d / float(1e-3);
                draw_nan_masked_grayscale(hr0_1d_im, 0, 1).save_to_file("hr0_1d_im.png");
                draw_nan_masked_grayscale(hr1_1d_im, 0, 1).save_to_file("hr1_1d_im.png");
                hr0_1d = box_filter_nan(hr0_1d, ArrayShape{10, 10}, NAN);
                hr1_1d = box_filter_nan(hr1_1d, ArrayShape{10, 10}, NAN);
                for (size_t r = 0; r < hr0_1d.shape(0); ++r) {
                    for (size_t c = 0; c < hr0_1d.shape(1); ++c) {
                        if (!(hr0_1d(r, c) > 1e-4)) {
                            for (size_t d = 0; d < 3; ++d) {
                                im0_rgb(d, r, c) = NAN;
                            }
                            im0_gray_masked(r, c) = NAN;
                        } else {
                            for (size_t d = 0; d < 3; ++d) {
                                //im0_rgb(d, r, c) *= hr0_1d_im(r, c);
                            }
                        }
                        if (!(hr1_1d(r, c) > 1e-4)) {
                            for (size_t d = 0; d < 3; ++d) {
                                im1_rgb(d, r, c) = NAN;
                            }
                            im1_gray_masked(r, c) = NAN;
                        } else {
                            for (size_t d = 0; d < 3; ++d) {
                                //im1_rgb(d, r, c) *= hr1_1d_im(r, c);
                            }
                        }
                    }
                }
                draw_nan_masked_grayscale(im0_gray_masked, 0, 1).save_to_file("hrm-0.png");
                draw_nan_masked_grayscale(im1_gray_masked, 0, 1).save_to_file("hrm-1.png");
                draw_nan_masked_rgb(im0_rgb, 0, 1).save_to_file("im0_rgb.png");
                draw_nan_masked_rgb(im1_rgb, 0, 1).save_to_file("im1_rgb.png");
            }

            size_t search_length = 70;
            float worst_error = 0.3f;

            // disparity_0 = compute_disparity_gray_single_pixel(im0_gray, im1_gray, F, search_length);
            // disparity_0 = compute_disparity_rgb_patch(im0_rgb, im1_rgb, F, search_length, worst_error, ArrayShape{10, 10});
            disparity_0 = compute_disparity_rgb_patch(im0_rgb, im1_rgb, F, search_length, worst_error, FixedArray<size_t, 2>{15u, 15u}, FixedArray<size_t, 2>{0u, 0u}, &error_0);
            disparity_0.save_txt_2d("disparity_0.m");
            error_0.save_txt_2d("error_0.m");
            draw_nan_masked_grayscale(disparity_0, -50.f, 50.f).save_to_file("disparity-0.png");

            // Array<float> disparity_0_x = compute_disparity_rgb_patch(im0_rgb, im1_rgb, F, search_length, worst_error, ArrayShape{10, 10}, ArrayShape{5, 5});
            // draw_nan_masked_grayscale(disparity_0_x, -50.f, 50.f).save_to_file("disparity-0-x.png");

            // draw_nan_masked_grayscale(abs(disparity_0 - disparity_0_x), -5.f, 5.f).save_to_file("disparity-0-diff.png");

            // Array<float> disparity_1 = compute_disparity_gray_single_pixel(im1_gray, im0_gray, F.T(), search_length);
            // draw_nan_masked_grayscale("disparity-1.png", disparity_1, -250.f, 250.f);

            // Array<float> disparity_0_f = guided_filter(im0_gray, disparity_0, ArrayShape{15, 15}, float(1e-3));
            // Array<float> disparity_0_f = quantized(disparity_0, Array<float>{0, 20, 30, 40, 60, 70});
            // draw_nan_masked_grayscale(disparity_0_f, -50.f, 50.f).save_to_file("disparity-0-f.png");


            Array<float> im_01_rgb = move_along_disparity(disparity_0, F, im1_rgb);
            draw_nan_masked_rgb(im_01_rgb, 0, 1).save_to_file("im-01.png");

            // Array<float> im_01_rgb_smooth = move_along_disparity(disparity_0, F, im1_rgb_smooth);
            // draw_nan_masked_rgb(im_01_rgb_smooth, 0, 1).save_to_file("im-01s.png");

            draw_nan_masked_rgb(abs(im_01_rgb - im0_rgb), 0, 0.5).save_to_file("im-01-diff.png");

            draw_nan_masked_rgb(abs(box_filter_nan_multichannel(im_01_rgb - im0_rgb, ArrayShape{5, 5}, NAN)), 0, 0.5).save_to_file("im-01-sm-diff.png");

            // Array<float> disparity_0_f = guided_filter(im0_gray, disparity_0, ArrayShape{10, 10}, float(1e-3));

            x = reconstruct_disparity(disparity_0, F, intrinsic_matrix, ke, &condition_number);
            x[0].save_txt_2d("x-0.m");
            x[1].save_txt_2d("x-1.m");
            x[2].save_txt_2d("x-2.m");
            condition_number.save_txt_2d("condition_number.m");
        } else {
            std::cout << "condition_number.m exists, loading..." << std::endl;
            disparity_0 = Array<float>::load_txt_2d("disparity_0.m");
            error_0 = Array<float>::load_txt_2d("error_0.m");
            x = Array<float>{
                Array<float>::load_txt_2d("x-0.m"),
                Array<float>::load_txt_2d("x-1.m"),
                Array<float>::load_txt_2d("x-2.m")};
            condition_number = Array<float>::load_txt_2d("condition_number.m");
        }

        if (false) {
            Array<float> hr0_1d = harris_response_1d(im0_gray, F);
            for (size_t r = 0; r < hr0_1d.shape(0); ++r) {
                for (size_t c = 0; c < hr0_1d.shape(1); ++c) {
                    if (!(hr0_1d(r, c) > 1e-4)) {
                        for (size_t d = 0; d < 3; ++d) {
                            x(d, r, c) = NAN;
                        }
                        condition_number(r, c) = NAN;
                    }
                }
            }
        }

        if (false) {
            Array<bool> mask0 = StbImage3::load_from_file("mask/mask.png").to_float_grayscale().casted<bool>();
            for (size_t r = 0; r < mask0.shape(0); ++r) {
                for (size_t c = 0; c < mask0.shape(1); ++c) {
                    if (!mask0(r, c)) {
                        for (size_t d = 0; d < 3; ++d) {
                            x(d, r, c) = NAN;
                        }
                        condition_number(r, c) = NAN;
                    }
                }
            }
        }

        if (false) {
            Array<float> hr0_1d = harris_response_1d(im0_gray, F);
            Array<float> hr0_1d_im = hr0_1d / float(1e-3);
            draw_nan_masked_grayscale(hr0_1d_im, 0, 1).save_to_file("hr0_1d_im.png");
            for (size_t r = 0; r < hr0_1d.shape(0); ++r) {
                for (size_t c = 0; c < hr0_1d.shape(1); ++c) {
                    if (!(hr0_1d(r, c) > 1e-3)) {
                        x(0, r, c) = NAN;
                        x(1, r, c) = NAN;
                        x(2, r, c) = NAN;
                        condition_number(r, c) = NAN;
                        // disparity_0(r, c) = NAN;
                    }
                }
            }
            Array<size_t> hist;
            Array<float> bins;
            histogram(disparity_0[Mlib::isfinite(disparity_0)], hist, bins);
            lerr() << bins;
            lerr() << hist;
        }

        // check constraints: dx/dx > 0, z > 0
        if (false) {
            for (size_t axis = 0; axis < 1; ++axis) {
                Array<float> dx = (central_differences_1d(x[axis], 1 - axis) > 0.f).casted<float>();
                Array<float> ldx = box_filter_NWE(dx, ArrayShape{5, 5});
                draw_nan_masked_grayscale(ldx, 0, 1).save_to_file("ldx-" + std::to_string(axis) + ".png");
                for (size_t r = 0; r < ldx.shape(0); ++r) {
                    for (size_t c = 0; c < ldx.shape(1); ++c) {
                        if (!(ldx(r, c) > 0.9)) {
                            x(0, r, c) = NAN;
                            x(1, r, c) = NAN;
                            x(2, r, c) = NAN;
                            condition_number(r, c) = NAN;
                            // disparity_0(r, c) = NAN;
                        }
                    }
                }
            }
            {
                Array<float> sz = (x[2] > 0.f).casted<float>();
                Array<float> lz = box_filter_NWE(sz, ArrayShape{2, 2});
                draw_nan_masked_grayscale(lz, 0, 1).save_to_file("lx-2.png");
                for (size_t r = 0; r < lz.shape(0); ++r) {
                    for (size_t c = 0; c < lz.shape(1); ++c) {
                        if (!(lz(r, c) > 0.9)) {
                            x(0, r, c) = NAN;
                            x(1, r, c) = NAN;
                            x(2, r, c) = NAN;
                            condition_number(r, c) = NAN;
                            // disparity_0(r, c) = NAN;
                        }
                    }
                }
            }
        }

        // filter_distance_to_camera(x, condition_number, im0_gray);

        // Array<float> d = sqrt(squared(x[0]) + squared(x[1]) + squared(x[2]));
        // draw_nan_masked_grayscale(d, -5.f, 5.f).save_to_file("d.png");

        // Array<float> xf2 = clipped(guided_filter(im0_gray, x[2], ArrayShape{15, 15}, float(1e-3)), 0.f, 1.f);
        draw_nan_masked_grayscale(x[0], -5.f, 5.f).save_to_file("x-0.png");
        draw_nan_masked_grayscale(x[1], -5.f, 5.f).save_to_file("x-1.png");
        draw_nan_masked_grayscale(x[2], -5.f, 5.f).save_to_file("x-2.png");
        // draw_nan_masked_grayscale(xf2, -2.f, 2.f).save_to_file("xf-2.png");

        if (false) {
            size_t search_length = 70;
            float worse_error = (float)0.3;
            Array<float> disparity_0_i = iterate_disparity_rgb_patch(
                im0_gray,
                im0_rgb,
                im1_rgb,
                F,
                search_length,
                worse_error,
                FixedArray<size_t, 2>{15u, 15u},
                FixedArray<size_t, 2>{0u, 0u},
                nullptr,
                &disparity_0);
            draw_nan_masked_grayscale(disparity_0_i, -50.f, 50.f).save_to_file("disparity_0_i.png");
            draw_nan_masked_grayscale(disparity_0_i - disparity_0, -30.f, 30.f).save_to_file("disparity_0_i-diff.png");
        }

        if (false) {
            Array<float> prior = StbImage3::load_from_file("disparity_0_i.png").to_float_grayscale();
            Array<float> gf = guided_filter(im0_gray, prior, ArrayShape{15, 15}, float(1e-3));
            draw_nan_masked_grayscale(gf, 0, 0).save_to_file("disparity_0_i_gf.png");
        }

        if (true) {
            Array<float> disparity_0_f = guided_filter(im0_gray, disparity_0, ArrayShape{15, 15}, float(1e-3));
            draw_nan_masked_grayscale(disparity_0_f, -50.f, 50.f).save_to_file("disparity-0-f.png");
            draw_nan_masked_grayscale(disparity_0_f - disparity_0, -10.f, 10.f).save_to_file("disparity-0-f-diff.png");
            for (size_t r = 0; r < disparity_0_f.shape(0); ++r) {
                for (size_t c = 0; c < disparity_0_f.shape(1); ++c) {
                    if (!(std::abs(disparity_0_f(r, c) - disparity_0(r, c)) < 10)) {
                        x(0, r, c) = NAN;
                        x(1, r, c) = NAN;
                        x(2, r, c) = NAN;
                        condition_number(r, c) = NAN;
                    }
                }
            }
        }

        if (true) {
            Array<float> hr0_1d = harris_response_1d(im0_gray, F);
            draw_nan_masked_grayscale(hr0_1d, 0.f, (float)1e-3).save_to_file("hr0_1d.png");
            hr0_1d = box_filter_nan(hr0_1d, ArrayShape{10, 10}, NAN);
            for (size_t r = 0; r < hr0_1d.shape(0); ++r) {
                for (size_t c = 0; c < hr0_1d.shape(1); ++c) {
                    if (!(hr0_1d(r, c) > 1e-4)) {
                        x(0, r, c) = NAN;
                        x(1, r, c) = NAN;
                        x(2, r, c) = NAN;
                        condition_number(r, c) = NAN;
                    }
                }
            }
        }

        if (false) {
            draw_nan_masked_grayscale(condition_number, 1, 1000).save_to_file("cond.png");
            Array<float> bp = guided_filter(im0_gray, (x[2] < 0.f).casted<float>(), ArrayShape{5, 5}, float(1e-3));
            // Array<float> bp = (x[2] < 0.f).casted<float>();
            // Array<float> bp = (disparity_0 < 20.f).casted<float>();
            // Array<float> bp = box_filter_nan((!(sad_filter(disparity_0, NAN) < 0.5f)).casted<float>(), ArrayShape{5, 5}, NAN);
            draw_nan_masked_grayscale(bp, 0, 1).save_to_file("bp.png");

            for (size_t r = 0; r < bp.shape(0); ++r) {
                for (size_t c = 0; c < bp.shape(1); ++c) {
                    if (bp(r, c) > 0.01) {
                        x(0, r, c) = NAN;
                        x(1, r, c) = NAN;
                        x(2, r, c) = NAN;
                        condition_number(r, c) = NAN;
                    }
                }
            }
        }

        if (false) {
            Array<float> md = median_filter_2d(disparity_0, 10, 20);
            Array<float> adiff = abs(md - disparity_0);
            draw_nan_masked_grayscale(adiff, 0, 0).save_to_file("mdm.png");
            Array<float> adiff2 = median_filter_2d(adiff, 10, 20);
            draw_nan_masked_grayscale(adiff2, 0, 0).save_to_file("mdm2.png");
            for (size_t r = 0; r < md.shape(0); ++r) {
                for (size_t c = 0; c < md.shape(1); ++c) {
                    // lerr() << std::abs(md(r, c) - disparity_0(r, c));
                    //if (std::abs(md(r, c) - disparity_0(r, c)) >= 1 || x(2, r, c) < 0) {
                    if (adiff2(r, c) >= 2) {
                        x(0, r, c) = NAN;
                        x(1, r, c) = NAN;
                        x(2, r, c) = NAN;
                        condition_number(r, c) = NAN;
                    }
                }
            }
        }

        // reproject(intrinsic_matrix, x, im0_rgb);
        if (false) {
            ColorDescriptor desc(im0_rgb);

            // Array<float> dx = difference_filter_1d(x[0], NAN, 1);

            std::list<Array<float>> dbd;
            for (size_t r = 0; r < im0_rgb.shape(1); ++r) {
                // lerr() << r;
                for (size_t c = 0; c < im0_rgb.shape(2); ++c) {
                    if (r % 10 != 0 || c % 10 != 0) {
                        continue;
                    }
                    if (desc.can_compute(r, c)) {
                        if (x(2, r, c) < 0) {
                            dbd.push_back(desc(r, c));
                        }
                    }
                }
            }
            NeighborDb<float> db(dbd, true); // true = standardize
            for (size_t r = 0; r < im0_rgb.shape(1); ++r) {
                // lerr() << r;
                for (size_t c = 0; c < im0_rgb.shape(2); ++c) {
                    if (!desc.can_compute(r, c) ||
                        db.count(desc(r, c), 0.1f) > 0) {
                        x(0, r, c) = NAN;
                        x(1, r, c) = NAN;
                        x(2, r, c) = NAN;
                        condition_number(r, c) = NAN;
                    }
                }
            }
        }

        if (false) {
            for (auto xx : x) {
                xx = median_filter_2d(xx, 10, 20);
            }
            // median filter removes NANs
            condition_number = median_filter_2d(condition_number, 10, 20);
        }

        if (true) {
            draw_nan_masked_grayscale(error_0, 0, 0).save_to_file("error_0.png");
        }

        if (true) {
            Array<bool> mask = zeros<bool>(x[0].shape());
            StbImage3 im = StbImage3::from_float_rgb(im0_rgb);
            for (size_t r = 0; r < condition_number.shape(0); ++r) {
                for (size_t c = 0; c < condition_number.shape(1); ++c) {
                    if (x(2, r, c) < 0) {
                        x(0, r, c) = NAN;
                        x(1, r, c) = NAN;
                        x(2, r, c) = NAN;
                        condition_number(r, c) = NAN;
                        im(r, c) = Rgb24::nan();
                        mask(r, c) = true;
                    }
                }
            }
            im.save_to_file("zlt0.bmp");

            Array<bool> inverse_mask;
            Array<float> img = im1_rgb.copy();
            move_along_disparity(disparity_0, F, im1_rgb, &mask, &inverse_mask);
            for (size_t r = 0; r < inverse_mask.shape(0); ++r) {
                for (size_t c = 0; c < inverse_mask.shape(1); ++c) {
                    if (inverse_mask(r, c) == 1) {
                        img(0, r, c) = NAN;
                        img(1, r, c) = NAN;
                        img(2, r, c) = NAN;
                    }
                }
            }
            draw_nan_masked_rgb(img, 0, 1).save_to_file("inverse_mask.png");
        }

        if (false) {
            Array<float> xm = median_filter_2d(x[2], 15, 20);
            for (size_t r = 0; r < xm.shape(0); ++r) {
                for (size_t c = 0; c < xm.shape(1); ++c) {
                    if (!(std::abs(xm(r, c) - x(2, r, c)) < 50)) {
                        x(0, r, c) = NAN;
                        x(1, r, c) = NAN;
                        x(2, r, c) = NAN;
                        condition_number(r, c) = NAN;
                    }
                }
            }
        }

        if (false) {
            Array<float> disparity_diff = StbImage3::load_from_file("disparity-0-diff.png").to_float_grayscale();
            for (size_t r = 0; r < disparity_diff.shape(0); ++r) {
                for (size_t c = 0; c < disparity_diff.shape(1); ++c) {
                    if (disparity_diff(r, c) > 0.8) {
                        x(0, r, c) = NAN;
                        x(1, r, c) = NAN;
                        x(2, r, c) = NAN;
                        condition_number(r, c) = NAN;
                    }
                }
            }
        }
        if (false) {
            Array<float> diff01 = StbImage3::load_from_file("im-01-sm-diff.png").to_float_grayscale();
            for (size_t r = 0; r < diff01.shape(0); ++r) {
                for (size_t c = 0; c < diff01.shape(1); ++c) {
                    if (diff01(r, c) > 0.05) {
                        x(0, r, c) = NAN;
                        x(1, r, c) = NAN;
                        x(2, r, c) = NAN;
                        condition_number(r, c) = NAN;
                    }
                }
            }
        }

        draw_nan_masked_grayscale(x[0], 0, 0).save_to_file("xp-0.png");
        draw_nan_masked_grayscale(x[1], 0, 0).save_to_file("xp-1.png");
        draw_nan_masked_grayscale(x[2], 0, 0).save_to_file("xp-2.png");

        MarginalizedMap<std::map<std::chrono::milliseconds, CameraFrame>> cams;
        cams.insert(std::make_pair(std::chrono::milliseconds{0}, CameraFrame{ TransformationMatrix<float, float, 3>::identity() }));
        cams.insert(std::make_pair(std::chrono::milliseconds{5}, CameraFrame{ ke }));
        DenseProjector::from_image(cams, 0, 1, 2, x, condition_number, intrinsic_matrix, TransformationMatrix<float, float, 3>::identity(), im0_rgb).normalize(256).draw("dense-0-1.png");
        DenseProjector::from_image(cams, 0, 2, 1, x, condition_number, intrinsic_matrix, TransformationMatrix<float, float, 3>::identity(), im0_rgb).normalize(256).draw("dense-0-2.png");
        DenseProjector::from_image(cams, 2, 1, 0, x, condition_number, intrinsic_matrix, TransformationMatrix<float, float, 3>::identity(), im0_rgb).normalize(256).draw("dense-2-1.png");

        return 0;
    } catch (const CommandLineArgumentError& e) {
        lerr() << e.what();
        return 1;
    }
}
