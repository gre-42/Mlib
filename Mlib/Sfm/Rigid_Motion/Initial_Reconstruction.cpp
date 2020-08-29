#include "Initial_Reconstruction.hpp"
#include <Mlib/Cv/Project_Points.hpp>
#include <Mlib/Geometry/Homogeneous.hpp>
#include <Mlib/Sfm/Rigid_Motion/Initial_Reconstruction2.hpp>

using namespace Mlib;
using namespace Mlib::Cv;
using namespace Mlib::Sfm;

InitialReconstruction::InitialReconstruction(
    const Array<float>& y0,
    const Array<float>& y1,
    const Array<float>& R,
    const Array<float>& t,
    const Array<float>& ki)
: y0(y0),
  y1(y1),
  R(R),
  t(t),
  ki(ki)
{}

Array<float> InitialReconstruction::reconstructed(Array<float>* condition_number) const {
    return initial_reconstruction(
        R,
        t,
        ki,
        y0,
        y1,
        false,  // points_are_normalized
        condition_number);
}

Array<float> InitialReconstruction::projection_residual0() const {
    Array<float> y0_pr = projected_points_1ke(
        homogenized_Nx4(reconstructed()),
        ki,
        assemble_inverse_homogeneous_3x4(identity_array<float>(3), zeros<float>(ArrayShape{3})));
    return dehomogenized_Nx2(y0_pr - y0, 0);
}

Array<float> InitialReconstruction::projection_residual1() const {
    Array<float> y1_pr = projected_points_1ke(
        homogenized_Nx4(reconstructed()),
        ki,
        assemble_inverse_homogeneous_3x4(R, t));
    return dehomogenized_Nx2(y1_pr - y1, 0);
}
