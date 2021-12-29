#include "Barycentric_Coordinates.hpp"
#include <Mlib/Geometry/Coordinates/Normalized_Points_Fixed.hpp>
#include <Mlib/Geometry/Mesh/Triangle_Exception.hpp>
#include <Mlib/Math/Fixed_Math.hpp>
#include <Mlib/Math/Transformation_Matrix.hpp>

using namespace Mlib;

/**
 * From: https://gamedev.stackexchange.com/a/23745/140709
 * Compute barycentric coordinates (u, v, w) for
 * point p with respect to triangle (a, b, c)
 */
void Mlib::barycentric(
    const FixedArray<float, 2>& p,
    const FixedArray<float, 2>& a,
    const FixedArray<float, 2>& b,
    const FixedArray<float, 2>& c,
    float &u,
    float &v,
    float &w)
{
    NormalizedPointsFixed np{ ScaleMode::DIAGONAL, OffsetMode::CENTERED };
    np.add_point(a);
    np.add_point(b);
    np.add_point(c);
    TransformationMatrix<float, 2> m = np.normalization_matrix();

    FixedArray<float, 2> an = m.transform(a);
    FixedArray<float, 2> bn = m.transform(b);
    FixedArray<float, 2> cn = m.transform(c);

    FixedArray<float, 2> v0 = (bn - an);
    FixedArray<float, 2> v1 = (cn - an);
    float d00 = dot0d(v0, v0);
    float d01 = dot0d(v0, v1);
    float d11 = dot0d(v1, v1);
    float denom = d00 * d11 - d01 * d01;
    // FixedArray<TData, 2, 2> M = dot2d(v0.columns_as_1D(), v1.rows_as_1D());
    // TData denom = dot0d(v0, dot1d(M - M.T(), v1));
    if (std::abs(denom) < 1e-14) {
        throw TriangleException(a, b, c, "barycentric coordinates encountered zero denominator");
    }
    FixedArray<float, 2> pn = m.transform(p);
    FixedArray<float, 2> v2 = (pn - an);
    float d20 = dot0d(v2, v0);
    float d21 = dot0d(v2, v1);
    v = (d11 * d20 - d01 * d21) / denom;
    w = (d00 * d21 - d01 * d20) / denom;
    u = 1 - v - w;
}
