#include "Fundamental_Matrix.hpp"
#include <Mlib/Geometry/Cross.hpp>
#include <Mlib/Geometry/Homogeneous.hpp>
#include <Mlib/Math/Math.hpp>
#include <Mlib/Math/Svd4.hpp>

using namespace Mlib;
using namespace Mlib::Sfm;

Array<float> Mlib::Sfm::find_fundamental_matrix(
    const Array<float>& y0,
    const Array<float>& y1,
    bool method_inverse_iteration)
{
    assert(y0.ndim() == 2);
    assert(y1.ndim() == 2);
    assert(y0.shape(1) == 3);
    assert(y1.shape(1) == 3);
    assert(all(y0.shape() == y1.shape()));
    Array<double> Y(ArrayShape{y0.shape(0), 9});
    for(size_t r = 0; r < y0.shape(0); ++r) {
        Y(r, 0) = y1(r, 0) * y0(r, 0); // e11
        Y(r, 1) = y1(r, 0) * y0(r, 1); // e12
        Y(r, 2) = y1(r, 0);            // e13
        Y(r, 3) = y1(r, 1) * y0(r, 0); // e21
        Y(r, 4) = y1(r, 1) * y0(r, 1); // e22
        Y(r, 5) = y1(r, 1);            // e23
        Y(r, 6) = y0(r, 0);            // e31
        Y(r, 7) = y0(r, 1);            // e32
        Y(r, 8) = 1;                   // e33
    }
    //Array<double> u;
    //Array<double> s;
    //Array<double> vT;
    //std::cerr << std::endl << "Y\n" << Y << std::endl;
    //svdcmp(Y, u, s, vT);
    //std::cerr << std::endl << vT.shape() << std::endl;
    //std::cerr << "s " << s << std::endl;
    //std::cerr << vT << std::endl;
    //std::cerr << (vT, vT[vT.shape(0) - 1]) << std::endl;
    //Array<float> F = vT[vT.shape(0) - 1].reshaped(ArrayShape{3, 3}).casted<float>();
    //std::cerr << "res\n" << (Y, vT[vT.shape(0) - 1]) << std::endl;
    //std::cerr << "v\n" << vT[vT.shape(0) - 1] << std::endl;
    if (method_inverse_iteration) {
        Array<double> u;
        double s;
        // If the inverse iteration fails, there are probably
        // multiple zero eigenvalues, which means that there
        // is a problem with the data.
        inverse_iteration_symm(dot(Y.T(), Y), u, s);
        return u.reshaped(ArrayShape{3, 3}).casted<float>();
    } else {
        Array<double> u;
        Array<double> vT;
        Array<double> s;
        svd4(dot(Y.T(), Y), u, s, vT);
        //std::cerr << "s " << s << std::endl;
        //std::cerr << "vT " << vT << std::endl;
        //std::cerr << vT.row_range(8, 9) << std::endl;
        return vT.row_range(8, 9).reshaped(ArrayShape{3, 3}).casted<float>();
        //std::cerr << vT << std::endl;
    }
    //std::cerr << "zzzz\n";
    //for(size_t i = 0; i < vT.shape(0); ++i) {
    //    F = vT[i].reshaped(ArrayShape{3, 3}).casted<float>();
    //    float z = 0;
    //    for(size_t r = 0; r < y0.shape(0); ++r) {
    //        // assert_isclose<float>((y1[r], (F, y0))(), 0, 5);
    //        z += squared((y1[r], (F, y0[r]))());
    //    }
    //    std::cerr << " " << z;
    //}
    //std::cerr << std::endl;
    //return F;
}

Array<float> Mlib::Sfm::fundamental_error(
    const Array<float>& F,
    const Array<float>& y0,
    const Array<float>& y1)
{
    assert(all(F.shape() == ArrayShape{3, 3}));
    assert(y0.ndim() == 2);
    assert(y0.shape(1) == 3);
    assert(all(y0.shape() == y1.shape()));
    Array<float> result(ArrayShape{y0.shape(0)});
    for(size_t r = 0; r < y1.shape(0); ++r) {
        result[r] = dot0d(y1[r], dot1d(F, y0[r]));
    }
    return result;
}

Array<float> Mlib::Sfm::fundamental_to_essential(const Array<float>& F, const Array<float>& intrinsic_matrix) {
    return dot(outer(intrinsic_matrix.T(), F), intrinsic_matrix);
    //return (lstsq_chol(intrinsic_matrix, F), intrinsic_matrix.T());
}

Array<float> Mlib::Sfm::find_epipole(const Array<float>& F) {
    Array<double> u, s, vT;
    svd4(F.casted<double>(), u, s, vT);
    return vT[1].casted<float>();
}

void Mlib::Sfm::find_epiline(
    const Array<float>& F,
    const Array<float>& y,
    Array<float>& p,
    Array<float>& v)
{
    assert(all(F.shape() == ArrayShape{3, 3}));
    assert(y.length() == 3);
    Array<float> n = dot1d(F, y);
    v = Array<float>{-n(1), n(0)};
    float c = n(2);
    Array<float> yd = dehomogenized_2(y);
    Array<float> nd = dehomogenized_2(n, n(2));
    // nd' * p + c = nd' * (yd + alpha * nd) + c = 0
    p = yd - nd * (dot0d(nd, yd) + c) / sum(squared(nd));
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
Array<float> Mlib::Sfm::fundamental_from_camera(
    const Array<float>& intrinsic0,
    const Array<float>& intrinsic1,
    const Array<float>& R,
    const Array<float>& t)
{
    assert(all(intrinsic0.shape() == ArrayShape{3, 3}));
    assert(all(intrinsic1.shape() == ArrayShape{3, 3}));
    assert(all(R.shape() == ArrayShape{3, 3}));
    assert(t.length() == 3);
    Array<float> Ri;
    Array<float> ti;
    invert_t_R(t, R, ti, Ri);
    return lstsq_chol(
        intrinsic1.T(),
        dot(outer(Ri, intrinsic0),
            cross(dot1d(outer(intrinsic0, Ri), ti))));
}
