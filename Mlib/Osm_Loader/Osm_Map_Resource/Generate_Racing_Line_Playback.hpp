#pragma once
#include <string>

namespace Mlib {

class GroundBvh;
template <class TData, size_t n>
class TransformationMatrix;

/** Convert a racing line to a playback by calculating the height and inserting zero angles.
 */
void generate_racing_line_playback(
    const std::string& racing_line_filename,
    const std::string& playback_filename,
    const TransformationMatrix<double, 2>& normalization_matrix,
    const TransformationMatrix<double, 3>& geographic_mapping,
    const GroundBvh& ground_bvh);

}
