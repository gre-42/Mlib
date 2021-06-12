#pragma once
#include <Mlib/Array/Array.hpp>

namespace Mlib {

void svd4(const Array<double>& a, Array<double>& u, Array<double>& s, Array<double>& vT);

double cond4(const Array<double>& a);

double cond4_x(const Array<double>& a);

Array<double> eig4(const Array<double>& a);

}
