#include "Essential_Matrix_To_TR.h"
#include <Mlib/Math/Math.hpp>
#include <Mlib/Math/Svd4.hpp>

using namespace Mlib;
using namespace Mlib::Sfm;

EssentialMatrixToTR::EssentialMatrixToTR(const Array<float>& E) {
    assert(all(E.shape() == ArrayShape{3, 3}));
    // From: Wikipedia, essential matrix
    //Array<float> uT;
    //Array<float> s;
    //Array<float> vT;
    //svd(E, uT, s, vT);
    Array<double> u;
    Array<double> s;
    Array<double> vT;
    svd4(E.casted<double>(), u, s, vT);
    //u = Array<double>({{-0.25851, -0.17774, 0.94952},
    //                   {0.83392, -0.53719, .12648},
    //                   {0.48759, 0.82452, 0.28709}});
    //vT = Array<double>({{0.17801,-0.25069, 0.95156},
    //                    {0.53480, 0.83637, 0.12030},
    //                    {-0.82601, 0.48748, 0.28295}}).T();
    Array<double> Z{{0, -1, 0}, {1, 0, 0}, {0, 0, 0}};
    Array<double> invW{{0, 1, 0}, {-1, 0, 0}, {0, 0, 1}};
    Array<double> W{{0, -1, 0}, {1, 0, 0}, {0, 0, 1}};
    Array<double> T = outer(dot(u, Z), u);
    Array<double> tu;
    double ts;
    inverse_iteration_symm(outer(T, T), tu, ts, 1e-12);
    //std::cerr << "E\n" << E << std::endl;
    //std::cerr << "u\n" << u << std::endl;
    //std::cerr << "s " << s << std::endl;
    //std::cerr << "vT\n" << vT << std::endl;
    //std::cerr << "T\n" << T << " tu " << tu << std::endl;

    t0 = tu.casted<float>();
    t1 = -tu.casted<float>();
    R0 = dot(dot(u, invW), vT).casted<float>();
    R1 = dot(dot(u, W), vT).casted<float>();
    //std::cerr << "E\n" << E << std::endl;
    //std::cerr << "u\n" << u << std::endl;
    //std::cerr << "vT\n" << vT << std::endl;
    //t0 *= det3x3(R0);
    //t1 *= det3x3(R1);
    //std::cerr << "R0\n" << R0 << std::endl;
    //std::cerr << "R1\n" << R1 << std::endl;
    R0 *= det3x3(R0);
    R1 *= det3x3(R1);
    if (std::abs(det3x3(R0) - 1) > 1e-6) {
        throw std::runtime_error("det(R0) not approx 1");
    }
    if (std::abs(det3x3(R1) - 1) > 1e-6) {
        throw std::runtime_error("det(R1) not approx 1");
    }
}
