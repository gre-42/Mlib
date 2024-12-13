#include "Synthetic_Scene.hpp"
#include <Mlib/Cv/Project_Points.hpp>
#include <Mlib/Geometry/Coordinates/Homogeneous.hpp>
#include <Mlib/Images/Bgr565Bitmap.hpp>
#include <Mlib/Images/Coordinates_Fixed.hpp>
#include <Mlib/Math/Math.hpp>

using namespace Mlib;
using namespace Mlib::Cv;
using namespace Mlib::Sfm;

static TransformationMatrix<float, float, 3> random_ke(unsigned int seed, float multiplier = 1.f) {
    FixedArray<float, 6> kep = uninitialized;
    kep.template row_range<0, 3>() = multiplier * (FixedArray<float, 3>{ random_array2<float>(ArrayShape{ 3 }, seed) } - 0.5f) * float(1e-1);
    kep.template row_range<3, 6>() = multiplier * (FixedArray<float, 3>{ random_array2<float>(ArrayShape{ 3 }, seed + 2348) } - 0.5f) * float(2e-1);
    return k_external(kep);
}

SyntheticScene::SyntheticScene(
    bool zero_first_extrinsic,
    float tR_multiplier)
// random points in 3D-space to be projected
:x(Array<float>::from_dynamic<3>(
    Array<float>({
        random_array2<float>(ArrayShape{20}, 483),
        random_array2<float>(ArrayShape{20}, 484),
        (2.f * ones<float>(ArrayShape{20}) +
         random_array2<float>(ArrayShape{20}, 485)) }).T())),
// an intrinsic camera-matrix
 ki{FixedArray<float, 3, 3>::init(
    256.f, 0.f, 200.f,
    0.f, 512.f, 490.f,
    0.f, 0.f, 1.f)},
// several extrinsic camera-matrices observing "x"
 ke({
    random_ke(1, (zero_first_extrinsic ? 0.f : 1.f) * tR_multiplier),
    random_ke(2, tR_multiplier),
    random_ke(3, tR_multiplier),
    random_ke(4, tR_multiplier),
    random_ke(5, tR_multiplier),
 }),
// "x" projected into the camera, using "ki" and "ke"
 y(projected_points(x, ki, ke)) {}

    TransformationMatrix<float, float, 3> SyntheticScene::delta_ke(size_t index0, size_t index1) {
    return inverse_projection_in_reference(ke(index0), ke(index1));
}

FixedArray<float, 3> SyntheticScene::dt2(size_t index0, size_t index1) {
    FixedArray<float, 3> tt = dt(index0, index1);
    return tt / float(std::sqrt(sum(squared(tt))));
}

FixedArray<float, 3> SyntheticScene::dt(size_t index0, size_t index1) {
    return delta_ke(index0, index1).t;
}

FixedArray<float, 3, 3> SyntheticScene::dR(size_t index0, size_t index1) {
    return delta_ke(index0, index1).R;
}

void SyntheticScene::draw_to_bmp(const std::string& filename, size_t index0, size_t index1) {
    Bgr565Bitmap bmp(ArrayShape{1024, 512}, Bgr565::white());
    for (size_t r = 0; r < x.shape(0); ++r) {
        const FixedArray<float, 2>& y0r = y(index0, r);
        const FixedArray<float, 2>& y1r = y(index1, r);
        FixedArray<size_t, 2> index0{a2i(y0r)};
        bmp.draw_fill_rect(index0, 3, Bgr565::red());

        FixedArray<size_t, 2> index1{a2i(y1r)};
        bmp.draw_fill_rect(index1, 3, Bgr565::blue());

        bmp.draw_infinite_line(
            FixedArray<float, 2>{y0r(id1), y0r(id0)},
            FixedArray<float, 2>{y1r(id1), y1r(id0)},
            0,
            Bgr565::green());
    }
    bmp.save_to_file(filename);
}
