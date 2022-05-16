#pragma once
#include <string>

namespace Mlib {

class RacingLineBvh;
template <class TData, size_t n>
class TransformationMatrix;

void load_racing_line_bvh(
    const std::string& filename,
    const TransformationMatrix<double, 2>& normalization_matrix,
    RacingLineBvh& racing_line_bvh);

}
