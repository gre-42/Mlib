#include "Detect_Chessboard.hpp"
#include <Mlib/Geometry/Homogeneous.hpp>
#include <Mlib/Images/Coordinates.hpp>
#include <Mlib/Images/Coordinates_Fixed.hpp>
#include <Mlib/Images/Features.hpp>
#include <Mlib/Images/PpmImage.hpp>
#include <Mlib/Math/Math.hpp>
#include <Mlib/Sfm/Homography/Apply_Homography.hpp>
#include <Mlib/Sfm/Homography/Homography_From_Points.hpp>

using namespace Mlib;
using namespace Mlib::Sfm;

class HomographyFeatureGrid {
public:
    explicit HomographyFeatureGrid(const ArrayShape& shape)
    : shape(shape) {}
    template <class TOperation>
    void foreach(const Array<float>& homography, const TOperation& op) const {
        for(size_t r = 0; r < shape(0); ++r) {
            for(size_t c = 0; c < shape(1); ++c) {
                Array<float> p0 = homogenized_3(i2a(ArrayShape{r, c}));
                op(p0, apply_homography(homography, p0));
            }
        }
    }
private:
    ArrayShape shape;
};

template <class TOperation>
void homography_pixel_grid(
    const HomographyFeatureGrid& fgrid,
    const ArrayShape& image_shape,
    const Array<float>& homography,
    const TOperation& op)
{
    fgrid.foreach(homography, [&](const Array<float>& p0, const Array<float>& pos){
        ArrayShape ipos{a2i(dehomogenized_2(pos))};
        if (all(ipos < image_shape)) {
            op(ipos);
        }
    });
}

class HomographyList {
public:
    HomographyList(const ArrayShape& shape, const Array<float>& feature_points, const float max_dist)
    : shape_{shape}, max_dist_{max_dist}
    {
        for(const Array<float>& p : feature_points) {
            feature_points_.push_back(FixedArray<float, 2>{p});
        }
    }

    template <class TOperation>
    void foreach(const Array<float>& homography, const TOperation& op) const {
        FixedArray<float, 3, 3> inverse_homography{inv(homography)};
        // std::cerr << homography << std::endl;

        for(const FixedArray<float, 2>& p : feature_points_) {
            FixedArray<float, 3> pos = apply_homography(inverse_homography, homogenized_3(p));
            FixedArray<size_t, 2> ip0{a2i(p)};
            FixedArray<size_t, 2> ipos{a2i(dehomogenized_2(pos))};
            float dist =
                squared(ipos(0) + 0.5f - pos(id1)) +
                squared(ipos(1) + 0.5f - pos(id0));
            // std::cerr << ipos << std::endl;
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
    Array<float>& p_x,
    Array<float>& p_y,
    PpmImage& bmp)
{
    assert(shape.ndim() == 2);
    bmp = PpmImage::from_float_grayscale(image);

    // Array<float> det = hessian_determinant_image(image);
    // result = Bgr565Bitmap::from_float_grayscale((det + 1.f) / 2.f);
    Array<float> feature_points = find_saddle_points(image);
    highlight_features(feature_points, bmp);

    HomographyList homography_list(shape, feature_points, 0.1);

    // ws: step-size of w
    const float ws = std::min(image.shape(0), image.shape(1)) / 128.f;
    const float xs = std::min(image.shape(0), image.shape(1)) / 128.f;
    const float ys = std::min(image.shape(0), image.shape(1)) / 128.f;

    float best_good = 0;
    Array<float> best_homography;
    for(float w = 3 * ws; w < 10 * ws; w += 0.5 * ws) {

        if (any(image.shape() < w * shape)) {
            throw std::runtime_error("Shape problem, internal error");
        }
        float maxX = image.shape(id1) - w * shape(id1);
        float maxY = image.shape(id0) - w * shape(id0);

        for(float fx = 0; fx < maxX; fx += 3 * xs) {
            for(float fy = 0; fy < maxY; fy += 3 * ys) {
                Array<unsigned int> hist = zeros<unsigned int>(shape);
                Array<float> homography{
                    {w, 0, fx},
                    {0, w, fy},
                    {0, 0, 1}};
                homography_list.foreach(homography, [&](
                    const FixedArray<size_t, 2>& ip0,
                    const FixedArray<size_t, 2>& ipos,
                    const FixedArray<float, 2>& fp0,
                    const FixedArray<float, 2>& fpos)
                {
                    hist(ipos(0), ipos(1)) = 1;
                });
                // std::cerr << homography << std::endl;

                float ngood = sum(hist);
                if (ngood > best_good) {
                    best_good = ngood;
                    best_homography = homography;
                    std::cerr << ngood << " w " << w << " x " << fx << " y " << fy << std::endl;
                }
            }
        }
    }
    {
        std::list<Array<float>> lst_x;
        std::list<Array<float>> lst_p;
        homography_list.foreach(best_homography, [&](
            const FixedArray<size_t, 2>& ip0,
            const FixedArray<size_t, 2>& ipos,
            const FixedArray<float, 2>& fp0,
            const FixedArray<float, 2>& fpos)
        {
            bmp(ip0(0), ip0(1)) = Rgb24::blue();
            lst_x.push_back(homogenized_3(Array<float>{fpos(0), fpos(1)}));
            lst_p.push_back(homogenized_3(Array<float>{fp0(0), fp0(1)}));
        });
        best_homography = homography_from_points(
            Array<float>(lst_x),
            Array<float>(lst_p));
    }
    p_x = Array<float>(ArrayShape{shape.nelements(), 4});
    p_y = Array<float>(ArrayShape{shape.nelements(), 3});
    HomographyFeatureGrid fg(shape);
    {
        size_t i = 0;
        fg.foreach(best_homography, [&](Array<float> p0, const Array<float>& pos) {
            assert(all(pos.shape() == ArrayShape{3}));
            p_x[i] = homogenized_4(p0);
            p_y[i] = pos;
            ++i;
        });
    }
    homography_pixel_grid(fg, image.shape(), best_homography, [&](const ArrayShape& ipos) {
        bmp(ipos) = Rgb24::green();
    });

    /*std::list<Array<float>> feature_points = find_saddle_points(image);
    std::cerr << feature_points.size() << std::endl;
    size_t i = 0;
    for(const Array<float>&p0 : feature_points) {
        ++i;
        size_t j = 0;
        for(const Array<float>&p1 : feature_points) {
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
            //std::cerr << (line, p0)() << " .... " << (line, p0)() << std::endl;
            size_t matches = 0;
            for(const Array<float>&p2 : feature_points) {
                if (std::abs((line, p2)() - c) < 3) {
                    ++matches;
                    matched.push_back(p2);
                    //std::cerr << (line, p2)() << " asd" << std::endl;
                }
            }
            if (matches > 10) {
                std::cerr << "match " << matches << " " << c << " " << line << " " << i << " " << j << " " << p0 << " " << p1 << std::endl;
                highlight_features(matched, result);
                //return result;
            }
        }
    }
    std::cerr << "done" << std::endl;
    */
    // highlight_features(feature_points, result);
}
