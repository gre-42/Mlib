#include "Eigen_Jacobi.hpp"
#include <Mlib/Array/Array.hpp>
#include <Mlib/Math/Eigen_Jacobi_Generic.hpp>

using namespace Mlib;

void Mlib::eigs_symm(const Array<double>& m, Array<double>& evals, Array<double>& evecs) {
    assert(m.ndim() == 2);
    assert(m.shape(0) == m.shape(1));
    if (m.shape(0) > INT_MAX) {
        THROW_OR_ABORT("Matrix too large");
    }
    jacobi_pd::Jacobi<double, Array<double>, Array<double>, const Array<double>&, Array<int>> eigen_calc((int)m.shape(0));
    evals.resize(m.shape(0));
    evecs.resize(m.shape());
    eigen_calc.Diagonalize(m, evals, evecs);
}

Array<double> Mlib::eigs_symm(const Array<double>& m) {
    Array<double> evals;
    Array<double> evecs;
    eigs_symm(m, evals, evecs);
    return evals;
}
