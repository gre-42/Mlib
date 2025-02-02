#pragma once

namespace Mlib {

template <class TData>
class Array;

void eigs_symm(const Array<double>& m, Array<double>& evals, Array<double>& evecs);
Array<double> eigs_symm(const Array<double>& m);

}
