#pragma once
#include <string>

namespace Mlib {

class GroundBvh;
template <class TDir, class TPos, size_t n>
class TransformationMatrix;
enum class FileStorageType;

/** Convert a racing line to a playback by calculating the height and inserting zero angles.
 */
void generate_racing_line_playback(
    const std::string& racing_line_filename,
    const std::string& playback_filename,
    const TransformationMatrix<double, double, 2>& normalization_matrix,
    const TransformationMatrix<double, double, 3>& geographic_mapping,
    const GroundBvh& ground_bvh,
    FileStorageType file_storage_type);

}
