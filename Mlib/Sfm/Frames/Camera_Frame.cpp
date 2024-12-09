#include "Camera_Frame.hpp"
#include <Mlib/Cv/Project_Points.hpp>
#include <Mlib/Geometry/Coordinates/Homogeneous.hpp>
#include <Mlib/Math/Rodrigues.hpp>
#include <Mlib/Sfm/Rigid_Motion/Initial_Reconstruction2.hpp>

using namespace Mlib;
using namespace Mlib::Cv;
using namespace Mlib::Sfm;

CameraFrame::CameraFrame(const TransformationMatrix<float, float, 3>& pose)
    : pose{ pose }
    , kep{ uninitialized }
{
    calculate_kep();
}

CameraFrame::CameraFrame(
    const TransformationMatrix<float, float, 3>& pose,
    const FixedArray<float, 6>& kep)
    : pose{ pose }
    , kep{ kep }
{}

TransformationMatrix<float, float, 3> CameraFrame::projection_matrix_3x4() const {
    return pose.inverted();
}

const TransformationMatrix<float, float, 3>& CameraFrame::reconstruction_matrix_3x4() const {
    return pose;
}

bool CameraFrame::point_in_fov(
    const FixedArray<float, 3>& x,
    const FixedArray<float, 2>& fov_distances) const
{
    float z = projection_matrix_3x4().transform(x)(2);
    return (z > fov_distances(0)) && (z < fov_distances(1));
}

FixedArray<float, 3> CameraFrame::dir(size_t i) const {
    return pose.R.column(i);
}

void CameraFrame::set_from_projection_matrix_3x4(const TransformationMatrix<float, float, 3>& projection)
{
    pose = projection.inverted();
    calculate_kep();
}

void CameraFrame::set_from_projection_matrix_3x4(
    const TransformationMatrix<float, float, 3>& projection,
    const FixedArray<float, 6>& kep)
{
    pose = projection.inverted();
    this->kep = kep;
}

void CameraFrame::calculate_kep() {
    kep = k_external_inverse(projection_matrix_3x4());
}
