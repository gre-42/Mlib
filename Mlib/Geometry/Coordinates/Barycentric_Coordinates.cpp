#include "Barycentric_Coordinates.hpp"
#include <Mlib/Geometry/Coordinates/Normalized_Points_Fixed.hpp>
#include <Mlib/Geometry/Exceptions/Triangle_Exception.hpp>
#include <Mlib/Math/Fixed_Math.hpp>
#include <Mlib/Math/Transformation/Transformation_Matrix.hpp>
#include <Mlib/Throw_Or_Abort.hpp>
#include <sstream>

using namespace Mlib;

/**
 * From: https://gamedev.stackexchange.com/a/23745/140709
 * Compute barycentric coordinates (u, v, w) for
 * point p with respect to triangle (a, b, c)
 */
void Mlib::barycentric(
    const FixedArray<double, 2>& p,
    const FixedArray<double, 2>& a,
    const FixedArray<double, 2>& b,
    const FixedArray<double, 2>& c,
    double &u,
    double &v,
    double &w)
{
    NormalizedPointsFixed<double> np{ ScaleMode::DIAGONAL, OffsetMode::CENTERED };
    np.add_point(a);
    np.add_point(b);
    np.add_point(c);
    TransformationMatrix<double, double, 2> m = np.normalization_matrix();

    FixedArray<double, 2> an = m.transform(a);
    FixedArray<double, 2> bn = m.transform(b);
    FixedArray<double, 2> cn = m.transform(c);

    FixedArray<double, 2> v0 = (bn - an);
    FixedArray<double, 2> v1 = (cn - an);
    double d00 = dot0d(v0, v0);
    double d01 = dot0d(v0, v1);
    double d11 = dot0d(v1, v1);
    double denom = d00 * d11 - d01 * d01;
    // FixedArray<TData, 2, 2> M = dot2d(v0.columns_as_1D(), v1.rows_as_1D());
    // TData denom = dot0d(v0, dot1d(M - M.T(), v1));
    if (std::abs(denom) < 1e-14) {
        auto sd = (std::stringstream() << denom).str();
        THROW_OR_ABORT2(TriangleException(a, b, c, "barycentric coordinates encountered zero denominator: " + sd));
    }
    FixedArray<double, 2> pn = m.transform(p);
    FixedArray<double, 2> v2 = (pn - an);
    double d20 = dot0d(v2, v0);
    double d21 = dot0d(v2, v1);
    v = (d11 * d20 - d01 * d21) / denom;
    w = (d00 * d21 - d01 * d20) / denom;
    u = 1 - v - w;
}
