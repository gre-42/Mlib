#include "Smallest_Eigenvector.hpp"
#include <Mlib/Math/Eigen_Jacobi.hpp>
#include <Mlib/Math/Svd4.hpp>

using namespace Mlib;

Array<double> Mlib::find_smallest_eigenvector_4(const Array<double>& m) {
    Array<double> u;
    Array<double> s;
    Array<double> vT;
    svd4(m, u, s, vT);
    return vT[vT.shape(0) - 1];
}

Array<double> Mlib::find_smallest_eigenvector_j(const Array<double>& m) {
    Array<double> evals;
    Array<double> evecs;
    eigs_symm(m, evals, evecs);
    return evecs[evecs.shape(0) - 1];
}
