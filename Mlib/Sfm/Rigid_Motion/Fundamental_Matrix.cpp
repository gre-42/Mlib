#include "Fundamental_Matrix.hpp"
#include <Mlib/Geometry/Coordinates/Homogeneous.hpp>
#include <Mlib/Geometry/Fixed_Cross.hpp>
#include <Mlib/Math/Fixed_Cholesky.hpp>
#include <Mlib/Math/Math.hpp>
#include <Mlib/Math/Power_Iteration/Inverse_Iteration.hpp>
#include <Mlib/Math/Smallest_Eigenvector.hpp>
#include <Mlib/Math/Svd4.hpp>
#include <Mlib/Math/Transformation/Transformation_Matrix.hpp>

using namespace Mlib;
using namespace Mlib::Sfm;

FixedArray<float, 3, 3> Mlib::Sfm::find_fundamental_matrix(
    const Array<FixedArray<float, 2>>& y0,
    const Array<FixedArray<float, 2>>& y1,
    bool method_inverse_iteration)
{
    assert(y0.ndim() == 1);
    assert(y1.ndim() == 1);
    assert(y0.length() == y1.length());
    Array<double> Y(ArrayShape{y0.length(), 9});
    // From: https://en.wikipedia.org/wiki/Eight-point_algorithm#Step_1:_Formulating_a_homogeneous_linear_equation
    for (size_t r = 0; r < y0.length(); ++r) {
        Y(r, 0) = double(y1(r)(0)) * y0(r)(0); // e11
        Y(r, 1) = double(y1(r)(0)) * y0(r)(1); // e12
        Y(r, 2) = y1(r)(0);                    // e13
        Y(r, 3) = double(y1(r)(1)) * y0(r)(0); // e21
        Y(r, 4) = double(y1(r)(1)) * y0(r)(1); // e22
        Y(r, 5) = y1(r)(1);                    // e23
        Y(r, 6) = y0(r)(0);                    // e31
        Y(r, 7) = y0(r)(1);                    // e32
        Y(r, 8) = 1;                           // e33
    }
    //Array<double> u;
    //Array<double> s;
    //Array<double> vT;
    //lerr() << std::endl << "Y\n" << Y;
    //svdcmp(Y, u, s, vT);
    //lerr() << std::endl << vT.shape();
    //lerr() << "s " << s;
    //lerr() << vT;
    //lerr() << (vT, vT[vT.shape(0) - 1]);
    //Array<float> F = vT[vT.shape(0) - 1].reshaped(ArrayShape{3, 3}).casted<float>();
    //lerr() << "res\n" << (Y, vT[vT.shape(0) - 1]);
    //lerr() << "v\n" << vT[vT.shape(0) - 1];
    if (method_inverse_iteration) {
        Array<double> u;
        double s;
        // If the inverse iteration fails, there are probably
        // multiple zero eigenvalues, which means that there
        // is a problem with the data.
        inverse_iteration_symm(dot(Y.T(), Y), u, s);
        return FixedArray<double, 3, 3>{u.reshaped(ArrayShape{ 3, 3 })}.casted<float>();
    } else {
        Array<double> ev = find_smallest_eigenvector_j(dot(Y.T(), Y));
        return FixedArray<double, 3, 3>{ev.reshaped(ArrayShape{ 3, 3 })}.casted<float>();
        //lerr() << vT;
    }
    //lerr() << "zzzz";
    //for (size_t i = 0; i < vT.shape(0); ++i) {
    //    F = vT[i].reshaped(ArrayShape{3, 3}).casted<float>();
    //    float z = 0;
    //    for (size_t r = 0; r < y0.shape(0); ++r) {
    //        // assert_isclose<float>((y1[r], (F, y0))(), 0, 5);
    //        z += squared((y1[r], (F, y0[r]))());
    //    }
    //    lerr(Decoration::PREFIX) << " " << z;
    //}
    //lerr();
    //return F;
}

Array<float> Mlib::Sfm::fundamental_error(
    const FixedArray<float, 3, 3>& F,
    const Array<FixedArray<float, 2>>& y0,
    const Array<FixedArray<float, 2>>& y1)
{
    assert(y0.ndim() == 1);
    assert(all(y0.shape() == y1.shape()));
    Array<float> result(ArrayShape{y0.shape(0)});
    for (size_t r = 0; r < y1.shape(0); ++r) {
        result(r) = dot0d(homogenized_3(y1(r)), dot1d(F, homogenized_3(y0(r))));
    }
    return result;
}

FixedArray<float, 3, 3> Mlib::Sfm::fundamental_to_essential(const FixedArray<float, 3, 3>& F, const TransformationMatrix<float, float, 2>& intrinsic_matrix) {
    return dot2d(dot2d(intrinsic_matrix.affine().T(), F), intrinsic_matrix.affine());
}

FixedArray<float, 2> Mlib::Sfm::find_epipole(const FixedArray<float, 3, 3>& F) {
    Array<double> u, s, vT;
    svd4(F.casted<double>().to_array(), u, s, vT);
    return FixedArray<double, 2>{
        vT(1, 0) / vT(1, 2),
        vT(1, 1) / vT(1, 2) }.casted<float>();
}

void Mlib::Sfm::find_epiline(
    const FixedArray<float, 3, 3>& F,
    const FixedArray<float, 2>& y,
    FixedArray<float, 2>& p,
    FixedArray<float, 2>& v)
{
    FixedArray<float, 3> n = dot1d(F, homogenized_3(y));
    v = FixedArray<float, 2>{-n(1), n(0)};
    float c = n(2);
    FixedArray<float, 2> nd = dehomogenized_2(n, n(2));
    // nd' * p + c = nd' * (yd + alpha * nd) + c = 0
    p = y - nd * (dot0d(nd, y) + c) / sum(squared(nd));
    /*float a = n_t(0);
    float b = n_t(1);
    float c = n_t(2);
    if (fabs(b) > fabs(a)) {
        Array<float> point1{0, -(c + a * 0) / b};
        Array<float> point2{1, -(c + a * 1) / b};
        return point2 - point1;
    } else {
        Array<float> point1{-(c + b * 0) / a, 0};
        Array<float> point2{-(c + b * 1) / a, 1};
        return point2 - point1;
    }*/
}

/**
 * Source: sourishgosh.com
 */
FixedArray<float, 3, 3> Mlib::Sfm::fundamental_from_camera(
    const TransformationMatrix<float, float, 2>& intrinsic0,
    const TransformationMatrix<float, float, 2>& intrinsic1,
    const TransformationMatrix<float, float, 3>& pose)
{
    TransformationMatrix<float, float, 3> ke = pose.inverted();
    return lstsq_chol(
        intrinsic1.affine().T(),
        dot2d(
            outer2d(ke.R, intrinsic0.affine()),
            cross(dot1d(outer(intrinsic0.affine(), ke.R), ke.t)))).value();
}
