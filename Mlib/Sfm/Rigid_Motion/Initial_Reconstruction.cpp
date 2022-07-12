#include "Initial_Reconstruction.hpp"
#include <Mlib/Cv/Project_Points.hpp>
#include <Mlib/Geometry/Coordinates/Homogeneous.hpp>
#include <Mlib/Sfm/Rigid_Motion/Initial_Reconstruction2.hpp>

using namespace Mlib;
using namespace Mlib::Cv;
using namespace Mlib::Sfm;

InitialReconstruction::InitialReconstruction(
    const Array<FixedArray<float, 2>>& y0,
    const Array<FixedArray<float, 2>>& y1,
    const TransformationMatrix<float, float, 3>& ke,
    const TransformationMatrix<float, float, 2>& ki)
: y0( y0 ),
  y1( y1 ),
  ke{ ke },
  ki{ ki }
{}

Array<FixedArray<float, 3>> InitialReconstruction::reconstructed(Array<float>* condition_number) const {
    return initial_reconstruction(
        ke,
        ki,
        y0,
        y1,
        false,  // points_are_normalized
        condition_number);
}

Array<FixedArray<float, 2>> InitialReconstruction::projection_residual0() const {
    Array<FixedArray<float, 2>> y0_pr = projected_points_1ke(
        reconstructed(),
        ki,
        TransformationMatrix<float, float, 3>::identity());
    return y0_pr - y0;
}

Array<FixedArray<float, 2>> InitialReconstruction::projection_residual1() const {
    Array<FixedArray<float, 2>> y1_pr = projected_points_1ke(
        reconstructed(),
        ki,
        ke);
    return y1_pr - y1;
}
