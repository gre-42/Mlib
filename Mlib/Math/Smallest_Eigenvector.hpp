#pragma once

namespace Mlib {

struct EigenVectorAndValue;
template <class TData>
class Array;

EigenVectorAndValue find_smallest_eigenvector_4(const Array<double>& m);
EigenVectorAndValue find_smallest_eigenvector_j(const Array<double>& m);

}
