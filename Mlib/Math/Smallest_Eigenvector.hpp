#pragma once

namespace Mlib {

template <class TData>
class Array;

Array<double> find_smallest_eigenvector_4(const Array<double>& m);
Array<double> find_smallest_eigenvector_j(const Array<double>& m);

}
