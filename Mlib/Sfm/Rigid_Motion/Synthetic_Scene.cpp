#include "Synthetic_Scene.hpp"
#include <Mlib/Cv/Project_Points.hpp>
#include <Mlib/Geometry/Homogeneous.hpp>
#include <Mlib/Images/Bgr565Bitmap.hpp>
#include <Mlib/Images/Coordinates.hpp>
#include <Mlib/Math/Math.hpp>

using namespace Mlib;
using namespace Mlib::Cv;
using namespace Mlib::Sfm;

static Array<float> random_ke(unsigned int seed, float multiplier = 1.f) {
    return k_external(Array<float>({
        multiplier * (random_array2<float>(ArrayShape{3}, seed) - 0.5f) * float(1e-1),
        multiplier * (random_array2<float>(ArrayShape{3}, seed + 2348) - 0.5f) * float(2e-1)}).flattened());
}

SyntheticScene::SyntheticScene(
    bool zero_first_extrinsic,
    float tR_multiplier)
// random points in 3D-space to be projected
:x(Array<float>({
    random_array2<float>(ArrayShape{20}, 483),
    random_array2<float>(ArrayShape{20}, 484),
    (2.f * ones<float>(ArrayShape{20}) +
     random_array2<float>(ArrayShape{20}, 485)),
    ones<float>(ArrayShape{20})}).T()),
// an intrinsic camera-matrix
 ki{{256, 0, 200},
    {0, 512, 490},
    {0, 0, 1}},
// several extrinsic camera-matrices observing "x"
 ke(std::list<Array<float>>{
    random_ke(1, (zero_first_extrinsic ? 0.f : 1.f) * tR_multiplier),
    random_ke(2, tR_multiplier),
    random_ke(3, tR_multiplier),
    random_ke(4, tR_multiplier),
    random_ke(5, tR_multiplier),
 }),
// "x" projected into the camera, using "ki" and "ke"
 y(projected_points(x, ki, ke)) {}

Array<float> SyntheticScene::delta_ke(size_t index0, size_t index1) {
    return inverse_projection_in_reference(ke[index0], ke[index1]);
}

Array<float> SyntheticScene::dt2(size_t index0, size_t index1) {
    Array<float> tt = dt(index0, index1);
    return tt / float(std::sqrt(sum(squared(tt))));
}

Array<float> SyntheticScene::dt(size_t index0, size_t index1) {
    return t3_from_Nx4(delta_ke(index0, index1), 4);
}

Array<float> SyntheticScene::dR(size_t index0, size_t index1) {
    return R3_from_Nx4(delta_ke(index0, index1), 4);
}

void SyntheticScene::draw_to_bmp(const std::string& filename, size_t index0, size_t index1) {
    Bgr565Bitmap bmp(ArrayShape{1024, 512}, Bgr565::white());
    for(size_t r = 0; r < x.shape(0); ++r) {
        const Array<float>& y0r = y[index0][r];
        const Array<float>& y1r = y[index1][r];
        ArrayShape index0{a2i(dehomogenized_2(y0r))};
        bmp.draw_fill_rect(index0, 3, Bgr565::red());

        ArrayShape index1{a2i(dehomogenized_2(y1r))};
        bmp.draw_fill_rect(index1, 3, Bgr565::blue());

        bmp.draw_infinite_line(
            Array<float>{y0r(id1), y0r(id0)},
            Array<float>{y1r(id1), y1r(id0)},
            0,
            Bgr565::green());
    }
    bmp.save_to_file(filename);
}
