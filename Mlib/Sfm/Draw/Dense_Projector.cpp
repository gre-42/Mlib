#include "Dense_Projector.hpp"
#include <Mlib/Cv/Project_Points.hpp>
#include <Mlib/Geometry/Normalized_Points.hpp>
#include <Mlib/Images/Draw_Generic.hpp>
#include <Mlib/Images/PpmImage.hpp>
#include <Mlib/Math/Math.hpp>
#include <Mlib/Stats/Robust.hpp>

using namespace Mlib;
using namespace Mlib::Cv;
using namespace Mlib::Sfm;

static const float condition_number_deviation_threshold = 0.5;

static const auto noop = [](size_t i, const ArrayShape& id){};

static Array<float> remove_nan_x_transposed(const Array<float>& x) {
    assert_true(x.ndim() == 3);
    assert_true(x.shape(0) == 3);
    Array<float> xx = x.columns_as_1D().T();
    Array<float> xv = xx[!Mlib::isnan(xx)];
    if (xv.nelements() % 3 != 0) {
        throw std::runtime_error("xv not divisible by 3");
    }
    xv.do_reshape(ArrayShape{xv.nelements() / 3, 3});
    return xv;
}

static Array<float> remove_nan_cond(const Array<float>& condition_number) {
    assert_true(condition_number.ndim() == 2);
    Array<float> cn = condition_number[!Mlib::isnan(condition_number)];
    return cn;
}

DenseProjector::DenseProjector(
    const MarginalizedMap<std::map<std::chrono::milliseconds, CameraFrame>>& camera_frames,
    size_t i0,
    size_t i1,
    size_t iz,
    const Array<float>& x,
    const Array<float>& condition_number,
    const Array<float>& ki,
    const Array<float>& ke,
    const Array<float>& rgb)
: ProjectorWithCameras{camera_frames, i0, i1, iz},
    x_{x},
    y_{dehomogenized_Nx2(projected_points_1ke(homogenized_Nx4(x_), ki, ke))},
    condition_number_{condition_number},
    rgb_{rgb}
{
    assert_true(x_.ndim() == 2);
    assert_true(x_.shape(1) == 3);
    if (condition_number.initialized() && (condition_number_.length() != x_.shape(0))) {
        throw std::runtime_error("cn and xv diverged");
    }
}

DenseProjector DenseProjector::from_image(
        const MarginalizedMap<std::map<std::chrono::milliseconds, CameraFrame>>& camera_frames,
        size_t i0,
        size_t i1,
        size_t iz,
        const Array<float>& x,
        const Array<float>& condition_number,
        const Array<float>& ki,
        const Array<float>& ke,
        const Array<float>& rgb)
{
    return DenseProjector{
        camera_frames,
        i0,
        i1,
        iz,
        remove_nan_x_transposed(x),
        condition_number.initialized()
            ? remove_nan_cond(condition_number)
            : condition_number,
        ki,
        ke,
        rgb};
}

template <class TOperation, class TElse>
void DenseProjector::for_each_point(
    const TOperation& op,
    const TElse& else_)
{
    //float mean_cond = mean(cond1d);
    Array<float> rd = condition_number_.initialized()
        ? robust_deviation(condition_number_)
        : zeros<float>(ArrayShape{x_.shape(0)});
    for (size_t xi = 0; xi < x_.shape(0); ++xi) {
        ArrayShape yid{a2i(y_[xi])};
        if (all(yid < rgb_.shape().erased_first())) {
            // if (condition_number_(i) < 2 * mean_cond) {
            if (rd(xi) < condition_number_deviation_threshold) {
                op(xi, yid);
            } else {
                else_(xi, yid);
            }
        } else {
            else_(xi, yid);
        }
    }
}

DenseProjector& DenseProjector::normalize(float scale) {
    NormalizedPoints npo(
        true,    // preserve_aspect_ratio
        false);  // centered
    for_each_point([&](size_t xi, const ArrayShape& yid) {
        npo.add_point(homogenized_3(project(x_[xi])));
    }, noop);
    for (const auto& c : camera_frames_) {
        npo.add_point(homogenized_3(project(c.second.position)));
    }
    scale_matrix_ = npo.normalization_matrix() * scale;
    return *this;
}

void DenseProjector::draw(const std::string& filename) {
    PpmImage ppm(ArrayShape{256, 256}, Rgb24::white());
    Array<float> depth = float(INFINITY) * ones<float>(ppm.shape());
    // float min_z = INFINITY;
    // float max_z = -INFINITY;
    // for_each_point([&](size_t i, const ArrayShape& id) {
    //     min_z = std::min(min_z, x_(i, 2));
    //     max_z = std::max(max_z, x_(i, 2));
    // }, noop);
    plot_camera_lines(ppm);
    for_each_point(
        [&](size_t xi, const ArrayShape& yid) {
            ArrayShape center = x2i(x_[xi]);
            float zz = zcoord(x_[xi]);
            if (any(center >= depth.shape()) || depth(center) < zz) {
                return;
            }
            draw_fill_rect(
                depth,
                center,
                1,
                zz);
            ppm.draw_fill_rect(
                center,
                1,
                Rgb24::from_float_rgb(
                    rgb_(0, yid(0), yid(1)),
                    rgb_(1, yid(0), yid(1)),
                    rgb_(2, yid(0), yid(1))));
                    // * (x_(i, 2) - min_z) / (max_z - min_z)
        }, [&](size_t i, const ArrayShape& id) {
            ppm.draw_fill_rect(
                x2i(x_[i]),
                1,
                Rgb24::green());
        });
    plot_camera_positions(ppm);
    ppm.save_to_file(filename);
}
