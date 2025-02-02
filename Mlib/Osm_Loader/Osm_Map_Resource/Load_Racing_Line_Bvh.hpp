#pragma once
#include <string>

namespace Mlib {

class RacingLineBvh;
template <class TDir, class TPos, size_t n>
class TransformationMatrix;

void load_racing_line_bvh(
    const std::string& filename,
    const TransformationMatrix<double, double, 2>& normalization_matrix,
    RacingLineBvh& racing_line_bvh);

}
