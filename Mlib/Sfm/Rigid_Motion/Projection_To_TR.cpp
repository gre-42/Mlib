#include "Projection_To_TR.hpp"
#include <Mlib/Array/Array.hpp>
#include <Mlib/Geometry/Homogeneous.hpp>
#include <Mlib/Sfm/Rigid_Motion/Fundamental_Matrix.hpp>
#include <Mlib/Sfm/Rigid_Motion/Initial_Reconstruction2.hpp>
#include <Mlib/Sfm/Rigid_Motion/Normalized_Projection.hpp>

using namespace Mlib;
using namespace Mlib::Sfm;

static bool reconstruction_ok(
    const Array<float>& R,
    const Array<float>& t,
    const Array<float>& ki,
    const Array<float>& y0,
    const Array<float>& y1,
    float threshold)
{
    assert(all(y0.shape() == y1.shape()));
    // std::cerr << "RRRR\n" << R << std::endl;
    // std::cerr << "ttttt\n" << t << std::endl;
    // std::cerr << "xxxxxx\n" << initial_reconstruction(R, t, ki, y0, y1) << std::endl;
    // return all(initial_reconstruction(R, t, ki, y0, y1).col_range(2, 3) > threshold);
    // auto x3 = initial_reconstruction_x3(R, t, ki, y0, y1);
    // auto ir = initial_reconstruction(R, t, ki, y0, y1);
    // std::cerr << "recon\n" << ir/(ir(0, 2)) << all(initial_reconstruction(R, t, ki, y0, y1) > threshold) << std::endl;
    // std::cerr << "x3\n" << x3/x3(0, 1) << std::endl;
    // size_t nz = count_nonzero(initial_reconstruction(R, t, ki, y0, y1).col_range(2, 3) > threshold);
    // return (float(nz) / y0.shape(0)) > 0.9;

    // return all(initial_reconstruction_x3(R, t, ki, y0, y1) > threshold);
    Array<float> recon0 = initial_reconstruction(R, t, ki, y0, y1);
    Array<float> recon1 = outer(assemble_inverse_homogeneous_3x4(R, t), homogenized_Nx4(recon0)).T();
    return all(recon0.col_range(2, 3) > threshold) &&
           all(recon1.col_range(2, 3) > threshold);
}

ProjectionToTR::ProjectionToTR(
    const Array<float>& y0,
    const Array<float>& y1,
    const Array<float>& intrinsic_matrix,
    float threshold)
: ngood(0),
  y(std::list<Array<float>>{y0, y1}),
  np(y),
  kin(np.normalized_intrinsic_matrix(intrinsic_matrix)),
  Fn(find_fundamental_matrix(np.yn[0], np.yn[1])),
  En(fundamental_to_essential(Fn, kin)),
  e2tr(En)
{
    const auto& v = e2tr;
    //std::cerr << v.R0 << std::endl;
    //std::cerr << v.R1 << std::endl;
    if (reconstruction_ok(v.R0, v.t0, kin, np.yn[0], np.yn[1], threshold)) {++ngood; t = v.t0; R = v.R0;}
    if (reconstruction_ok(v.R1, v.t0, kin, np.yn[0], np.yn[1], threshold)) {++ngood; t = v.t0; R = v.R1;}
    if (reconstruction_ok(v.R0, v.t1, kin, np.yn[0], np.yn[1], threshold)) {++ngood; t = v.t1; R = v.R0;}
    if (reconstruction_ok(v.R1, v.t1, kin, np.yn[0], np.yn[1], threshold)) {++ngood; t = v.t1; R = v.R1;}
}

bool ProjectionToTR::good() const {
    assert((ngood != 1) || (det3x3(R) > 0));
    return (ngood == 1);
}

InitialReconstruction ProjectionToTR::initial_reconstruction() const {
    return InitialReconstruction(np.yn[0], np.yn[1], R, t, kin);
}
