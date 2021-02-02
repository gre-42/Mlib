#include "Camera_Frame.hpp"
#include <Mlib/Cv/Project_Points.hpp>
#include <Mlib/Geometry/Homogeneous.hpp>
#include <Mlib/Math/Rodrigues.hpp>
#include <Mlib/Sfm/Rigid_Motion/Initial_Reconstruction2.hpp>

using namespace Mlib;
using namespace Mlib::Cv;
using namespace Mlib::Sfm;

CameraFrame::CameraFrame(
    const Array<float>& rotation,
    const Array<float>& position,
    const Array<float>& kep)
:rotation(rotation),
 position(position),
 kep(kep)
 {
    assert(all(rotation.shape() == ArrayShape{3, 3}));
    assert(all(position.shape() == ArrayShape{3}));
    assert_true(!kep.initialized() || all(kep.shape() == ArrayShape{6}));
    set_kep_if_undefined();
}

Array<float> CameraFrame::projection_matrix_3x4() const {
    return assemble_inverse_homogeneous_3x4(rotation, position);
}

Array<float> CameraFrame::reconstruction_matrix_3x4() const {
    return assemble_homogeneous_3x4(rotation, position);
}

bool CameraFrame::point_in_fov(const Array<float>& x, float threshold) const {
    assert(all(x.shape() == 3));
    return dot1d(projection_matrix_3x4(), homogenized_4(x))(2) > threshold;
}

Array<float> CameraFrame::dir(size_t i) const {
    return rotation.col_range(i, i + 1).flattened();
}

void CameraFrame::set_from_projection_matrix_3x4(
    const Array<float>& projection,
    const Array<float>& kep)
{
    assert_true(!kep.initialized() || all(kep.shape() == ArrayShape{6}));
    homogeneous_to_inverse_t_R(projection, position, rotation);
    this->kep = kep;
    set_kep_if_undefined();
}

void CameraFrame::set_kep_if_undefined() {
    if (!kep.initialized()) {
        kep = k_external_inverse(projection_matrix_3x4());
    }
}
