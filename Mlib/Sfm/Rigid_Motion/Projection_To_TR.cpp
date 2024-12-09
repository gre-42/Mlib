#include "Projection_To_TR.hpp"
#include <Mlib/Array/Array.hpp>
#include <Mlib/Geometry/Coordinates/Homogeneous.hpp>
#include <Mlib/Math/Fixed_Determinant.hpp>
#include <Mlib/Sfm/Draw/Epilines.hpp>
#include <Mlib/Sfm/Rigid_Motion/Fundamental_Matrix.hpp>
#include <Mlib/Sfm/Rigid_Motion/Initial_Reconstruction2.hpp>
#include <Mlib/Sfm/Rigid_Motion/Normalized_Projection.hpp>

using namespace Mlib;
using namespace Mlib::Sfm;

static bool reconstruction_ok(
    const TransformationMatrix<float, float, 3>& ke,
    const TransformationMatrix<float, float, 2>& ki,
    const Array<FixedArray<float, 2>>& y0,
    const Array<FixedArray<float, 2>>& y1,
    const FixedArray<float, 2>& fov_distances)
{
    assert(all(y0.shape() == y1.shape()));
    // lerr() << "RRRR\n" << R;
    // lerr() << "ttttt\n" << t;
    // lerr() << "xxxxxx\n" << initial_reconstruction(R, t, ki, y0, y1);
    // return all(initial_reconstruction(R, t, ki, y0, y1).col_range(2, 3) > threshold);
    // auto x3 = initial_reconstruction_x3(R, t, ki, y0, y1);
    // auto ir = initial_reconstruction(R, t, ki, y0, y1);
    // lerr() << "recon\n" << ir/(ir(0, 2)) << all(initial_reconstruction(R, t, ki, y0, y1) > threshold);
    // lerr() << "x3\n" << x3/x3(0, 1);
    // size_t nz = count_nonzero(initial_reconstruction(R, t, ki, y0, y1).col_range(2, 3) > threshold);
    // return (float(nz) / y0.shape(0)) > 0.9;

    // return all(initial_reconstruction_x3(R, t, ki, y0, y1) > threshold);
    Array<FixedArray<float, 3>> recon0 = initial_reconstruction(ke, ki, y0, y1);
    Array<FixedArray<float, 3>> recon1 = recon0.applied([&ke](const auto& p) { return ke.transform(p); });
    return all(recon0.applied<bool>([&fov_distances](const auto& p) { return (p(2) > fov_distances(0)) && (p(2) < fov_distances(1)); })) &&
           all(recon1.applied<bool>([&fov_distances](const auto& p) { return (p(2) > fov_distances(0)) && (p(2) < fov_distances(1)); }));
}

ProjectionToTR::ProjectionToTR(
    const Array<FixedArray<float, 2>>& y0,
    const Array<FixedArray<float, 2>>& y1,
    const TransformationMatrix<float, float, 2>& intrinsic_matrix,
    const FixedArray<float, 2>& fov_distances)
    : ke{ uninitialized }
    , ngood{ 0 }
    , np{ Array<FixedArray<float, 2>>{y0, y1} }
    , kin{ np.normalized_intrinsic_matrix(intrinsic_matrix) }
    , Fn{ find_fundamental_matrix(np.yn[0], np.yn[1]) }
    , En{ fundamental_to_essential(Fn, kin) }
    , e2tr{ En }
{
    const auto& v = e2tr;
    //lerr() << v.R0;
    //lerr() << v.R1;
    if (reconstruction_ok(TransformationMatrix<float, float, 3>{v.ke0.R, v.ke0.t}, kin, np.yn[0], np.yn[1], fov_distances)) { ++ngood; ke.R = v.ke0.R; ke.t = v.ke0.t; }
    if (reconstruction_ok(TransformationMatrix<float, float, 3>{v.ke1.R, v.ke0.t}, kin, np.yn[0], np.yn[1], fov_distances)) { ++ngood; ke.R = v.ke1.R; ke.t = v.ke0.t; }
    if (reconstruction_ok(TransformationMatrix<float, float, 3>{v.ke0.R, v.ke1.t}, kin, np.yn[0], np.yn[1], fov_distances)) { ++ngood; ke.R = v.ke0.R; ke.t = v.ke1.t; }
    if (reconstruction_ok(TransformationMatrix<float, float, 3>{v.ke1.R, v.ke1.t}, kin, np.yn[0], np.yn[1], fov_distances)) { ++ngood; ke.R = v.ke1.R; ke.t = v.ke1.t; }
}

bool ProjectionToTR::good() const {
    assert((ngood != 1) || (det3x3(ke.R()) > 0));
    return (ngood == 1);
}

InitialReconstruction ProjectionToTR::initial_reconstruction() const {
    return InitialReconstruction(np.yn[0], np.yn[1], ke, kin);
}

Array<float> ProjectionToTR::fundamental_error(
    const Array<FixedArray<float, 2>>& y0,
    const Array<FixedArray<float, 2>>& y1) const
{
    auto y0_n = np.normalized_y(y0);
    auto y1_n = np.normalized_y(y1);
    return Mlib::Sfm::fundamental_error(Fn, y0_n, y1_n) / np.N.get_scale();
}

void ProjectionToTR::draw_epilines(StbImage3& image, const Rgb24& color) const {
    draw_epilines_from_F(fundamental_to_essential(Fn, np.N), image, color);
}
