#include "Essential_Matrix_To_TR.h"
#include <Mlib/Math/Fixed_Determinant.hpp>
#include <Mlib/Math/Math.hpp>
#include <Mlib/Math/Smallest_Eigenvector.hpp>
#include <Mlib/Math/Svd4.hpp>

using namespace Mlib;
using namespace Mlib::Sfm;

EssentialMatrixToTR::EssentialMatrixToTR(const FixedArray<float, 3, 3>& E)
    : ke0{ uninitialized }
    , ke1{ uninitialized }
{
    // From: https://en.wikipedia.org/wiki/Essential_matrix#Extracting_rotation_and_translation

    //Array<float> uT;
    //Array<float> s;
    //Array<float> vT;
    //svd(E, uT, s, vT);
    Array<double> u;
    Array<double> s;
    Array<double> vT;
    svd4(E.casted<double>().to_array(), u, s, vT);
    //u = Array<double>({{-0.25851, -0.17774, 0.94952},
    //                   {0.83392, -0.53719, .12648},
    //                   {0.48759, 0.82452, 0.28709}});
    //vT = Array<double>({{0.17801,-0.25069, 0.95156},
    //                    {0.53480, 0.83637, 0.12030},
    //                    {-0.82601, 0.48748, 0.28295}}).T();
    Array<double> Z{
        {0, 1, 0},
        {-1, 0, 0},
        {0, 0, 0}};
    Array<double> invW{
        {0, 1, 0},
        {-1, 0, 0},
        {0, 0, 1}};
    Array<double> W{
        {0, -1, 0},
        {1, 0, 0},
        {0, 0, 1}};
    Array<double> T = outer(dot(u, Z), u);
    Array<double> tu;
    //double ts;
    //inverse_iteration_symm(outer(T, T), tu, ts, 1e-12);
    //tu = find_smallest_eigenvector_4(outer(T, T));
    tu = find_smallest_eigenvector_j(outer(T, T));
    //lerr() << "E\n" << E;
    //lerr() << "u\n" << u;
    //lerr() << "s " << s;
    //lerr() << "vT\n" << vT;
    //lerr() << "T\n" << T << " tu " << tu;

    ke0.t = FixedArray<float, 3>{ tu.casted<float>() };
    ke1.t = FixedArray<float, 3>{ -tu.casted<float>() };
    ke0.R = FixedArray<float, 3, 3>{ dot(dot(u, invW), vT).casted<float>() };
    ke1.R = FixedArray<float, 3, 3>{ dot(dot(u, W), vT).casted<float>() };
    //lerr() << "E\n" << E;
    //lerr() << "u\n" << u;
    //lerr() << "vT\n" << vT;
    //t0 *= det3x3(R0);
    //t1 *= det3x3(R1);
    //lerr() << "R0\n" << R0;
    //lerr() << "R1\n" << R1;
    ke0.R *= det3x3(ke0.R);
    ke1.R *= det3x3(ke1.R);
    if (std::abs(det3x3(ke0.R) - 1) > 1e-6) {
        throw std::runtime_error("det(R0) not approx 1");
    }
    if (std::abs(det3x3(ke1.R) - 1) > 1e-6) {
        throw std::runtime_error("det(R1) not approx 1");
    }
}
