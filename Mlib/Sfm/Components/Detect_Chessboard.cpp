#include "Detect_Chessboard.hpp"
#include <Mlib/Geometry/Coordinates/Homogeneous.hpp>
#include <Mlib/Images/Coordinates.hpp>
#include <Mlib/Images/Coordinates_Fixed.hpp>
#include <Mlib/Images/Features.hpp>
#include <Mlib/Images/StbImage3.hpp>
#include <Mlib/Math/Fixed_Cholesky.hpp>
#include <Mlib/Math/Math.hpp>
#include <Mlib/Sfm/Homography/Apply_Homography.hpp>
#include <Mlib/Sfm/Homography/Homography_From_Points.hpp>
#include <Mlib/Throw_Or_Abort.hpp>

using namespace Mlib;
using namespace Mlib::Sfm;

class HomographyFeatureGrid {
public:
    explicit HomographyFeatureGrid(const FixedArray<size_t, 2>& shape, const FixedArray<float, 2>& inc)
    : shape_{ shape },
      inc_{ inc }
    {}
    template <class TOperation>
    void foreach(const FixedArray<float, 3, 3>& homography, const TOperation& op) const {
        for (float r = 0; r <= shape_(0) - 1; r += inc_(0)) {
            for (float c = 0; c <= shape_(1) - 1; c += inc_(1)) {
                FixedArray<float, 2> p0 = fi2a(FixedArray<float, 2>{r, c});
                op(p0, apply_homography(homography, p0));
            }
        }
    }
private:
    FixedArray<size_t, 2> shape_;
    FixedArray<float, 2> inc_;
};

template <class TOperation>
void homography_pixel_grid(
    const HomographyFeatureGrid& fgrid,
    const FixedArray<size_t, 2>& image_shape,
    const FixedArray<float, 3, 3>& homography,
    const TOperation& op)
{
    fgrid.foreach(homography, [&](const FixedArray<float, 2>& p0, const FixedArray<float, 2>& pos){
        FixedArray<size_t, 2> ipos{a2i(pos)};
        if (all(ipos < image_shape)) {
            op(ipos);
        }
    });
}

void draw_homography_grid(const ArrayShape& grid_shape, const ArrayShape& image_shape, FixedArray<float, 3, 3>& homography, StbImage3& bmp) {
    {
        HomographyFeatureGrid fg(FixedArray<size_t, 2>{ grid_shape }, FixedArray<float, 2>{ 0.01f, 1.f });
        homography_pixel_grid(fg, FixedArray<size_t, 2>{image_shape(0), image_shape(1)}, homography, [&](const FixedArray<size_t, 2>& ipos) {
            bmp(ipos(0), ipos(1)) = Rgb24::yellow();
            });
    }
    {
        HomographyFeatureGrid fg(FixedArray<size_t, 2>{ grid_shape }, FixedArray<float, 2>{ 1.f, 0.01f });
        homography_pixel_grid(fg, FixedArray<size_t, 2>{image_shape(0), image_shape(1)}, homography, [&](const FixedArray<size_t, 2>& ipos) {
            bmp(ipos(0), ipos(1)) = Rgb24::nan();
            });
    }
}

class HomographyList {
public:
    HomographyList(const ArrayShape& shape, const Array<FixedArray<float, 2>>& feature_points, const float max_dist)
    : shape_{shape}, max_dist_{max_dist}
    {
        for (const FixedArray<float, 2>& p : feature_points.flat_iterable()) {
            feature_points_.push_back(p);
        }
    }

    template <class TOperation>
    void foreach(const FixedArray<float, 3, 3>& homography, const TOperation& op) const {
        FixedArray<float, 3, 3> inverse_homography{ inv(homography.casted<double>()).value().casted<float>() };
        // lerr() << homography;

        for (const FixedArray<float, 2>& p : feature_points_) {
            FixedArray<float, 2> pos = apply_homography(inverse_homography, p);
            FixedArray<size_t, 2> ip0{a2i(p)};
            FixedArray<size_t, 2> ipos{a2i(pos)};
            float dist =
                squared(ipos(0) + 0.5f - pos(id1)) +
                squared(ipos(1) + 0.5f - pos(id0));
            // lerr() << ipos;
            if (all(ipos < shape_) && (dist < max_dist_)) {
                op(
                    ip0,
                    ipos,
                    p,
                    i2a(ipos));
            }
        }
    }
private:
    FixedArray<size_t, 2> shape_;
    std::list<FixedArray<float, 2>> feature_points_;
    const float max_dist_;
};

void Mlib::Sfm::detect_chessboard(
    const Array<float>& image,
    const ArrayShape& shape,
    Array<FixedArray<float, 2>>& p_x,
    Array<FixedArray<float, 2>>& p_y,
    StbImage3& bmp)
{
    assert(shape.ndim() == 2);
    bmp = StbImage3::from_float_grayscale(image);

    // Array<float> det = hessian_determinant_image(image);
    // result = Bgr565Bitmap::from_float_grayscale((det + 1.f) / 2.f);
    Array<FixedArray<float, 2>> feature_points = Array<float>::from_dynamic<2>(find_saddle_points(image));
    highlight_features(feature_points, bmp);

    HomographyList homography_list(shape, feature_points, 0.1f);

    // How to tune the grid search parameters in case something does not work:
    // 1. Start the chessboard detector and denote the (suboptimal) homography.
    // 2. Overwrite the homography in the chessboard detector with the suboptimal
    //    one and tune it until it is optimal.
    // 3. Start the chessboard detector again and observe the output of
    //    std::cerr << ngood ...
    //    and find out why the optimal parameters are not reached.

    // ws: step-size of w
    const float ws = std::min(image.shape(0), image.shape(1)) / 128.f;
    const float xs = std::min(image.shape(0), image.shape(1)) / 128.f;
    const float ys = std::min(image.shape(0), image.shape(1)) / 128.f;

    unsigned int best_good = 0;
    FixedArray<float, 3, 3> best_homography = uninitialized;
    for (float w = 3 * ws; w < 15 * ws; w += 0.5f * ws) {

        float maxX = image.shape(id1) - w * (shape(id1) - 1);
        float maxY = image.shape(id0) - w * (shape(id0) - 1);
        if (maxX < 0 || maxY < 0) {
            THROW_OR_ABORT("Shape problem, internal error");
        }

        for (float fx = 0; fx < maxX; fx += 3 * xs) {
            for (float fy = 0; fy < maxY; fy += 3 * ys) {
                Array<unsigned int> hist = zeros<unsigned int>(shape);
                auto homography = FixedArray<float, 3, 3>::init(
                    w  , 0.f, fx,
                    0.f, w  , fy,
                    0.f, 0.f, 1.f);
                // homography = FixedArray<float, 3, 3>{
                //     33.f, 0.f, 121.25f,
                //     0.f, 33.f, 101.25f,
                //     0.f, 0.f, 1.f, };
                homography_list.foreach(homography, [&](
                    const FixedArray<size_t, 2>& ip0,
                    const FixedArray<size_t, 2>& ipos,
                    const FixedArray<float, 2>& fp0,
                    const FixedArray<float, 2>& fpos)
                {
                    hist(ipos(0), ipos(1)) = 1;
                });
                // lerr() << homography;

                unsigned int ngood = sum(hist);
                if (ngood > best_good) {
                    best_good = ngood;
                    best_homography = homography;
                    lerr() << ngood << " w " << w << " x " << fx << " y " << fy;
                    // homography_list.foreach(best_homography, [&](
                    //     const FixedArray<size_t, 2>& ip0,
                    //     const FixedArray<size_t, 2>& ipos,
                    //     const FixedArray<float, 2>& fp0,
                    //     const FixedArray<float, 2>& fpos)
                    //     {
                    //         bmp(ip0(0), ip0(1)) = Rgb24::yellow();
                    //     });
                }
            }
        }
    }
    {
        if (best_good == 0) {
            THROW_OR_ABORT("Could not find a good homography");
        }
        draw_homography_grid(shape, image.shape(), best_homography, bmp);
        std::list<FixedArray<float, 2>> lst_x;
        std::list<FixedArray<float, 2>> lst_p;
        homography_list.foreach(best_homography, [&](
            const FixedArray<size_t, 2>& ip0,
            const FixedArray<size_t, 2>& ipos,
            const FixedArray<float, 2>& fp0,
            const FixedArray<float, 2>& fpos)
        {
            bmp(ip0(0), ip0(1)) = Rgb24::blue();
            lst_x.push_back(fpos);
            lst_p.push_back(fp0);
        });
        best_homography = homography_from_points(
            Array<FixedArray<float, 2>>(lst_x),
            Array<FixedArray<float, 2>>(lst_p));
    }
    p_x.resize(ArrayShape{ shape.nelements() });
    p_y.resize(ArrayShape{ shape.nelements() });
    HomographyFeatureGrid fg(FixedArray<size_t, 2>{ shape }, FixedArray<float, 2>{ 1.f, 1.f });
    {
        size_t i = 0;
        fg.foreach(best_homography, [&](const FixedArray<float, 2>& p0, const FixedArray<float, 2>& pos) {
            p_x(i) = p0;
            p_y(i) = pos;
            ++i;
        });
    }
    homography_pixel_grid(fg, FixedArray<size_t, 2>{image.shape(0), image.shape(1)}, best_homography, [&](const FixedArray<size_t, 2>& ipos) {
        bmp(ipos(0), ipos(1)) = Rgb24::green();
    });

    /*std::list<Array<float>> feature_points = find_saddle_points(image);
    lerr() << feature_points.size();
    size_t i = 0;
    for (const Array<float>&p0 : feature_points) {
        ++i;
        size_t j = 0;
        for (const Array<float>&p1 : feature_points) {
            ++j;
            if (i == j) {
                continue;
            }
            std::list<Array<float>> matched;

            const Array<float> line_raw = p1 - p0;
            if (std::abs(line_raw(1)) < 2 * std::abs(line_raw(0))) {
                continue;
            }
            const Array<float> line{line_raw(1), -line_raw(0)};
            float c = (line, p0)();
            //lerr() << (line, p0)() << " .... " << (line, p0)();
            size_t matches = 0;
            for (const Array<float>&p2 : feature_points) {
                if (std::abs((line, p2)() - c) < 3) {
                    ++matches;
                    matched.push_back(p2);
                    //lerr() << (line, p2)() << " asd";
                }
            }
            if (matches > 10) {
                lerr() << "match " << matches << " " << c << " " << line << " " << i << " " << j << " " << p0 << " " << p1;
                highlight_features(matched, result);
                //return result;
            }
        }
    }
    lerr() << "done";
    */
    // highlight_features(feature_points, result);
}
