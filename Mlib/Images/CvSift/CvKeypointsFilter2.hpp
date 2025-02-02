#pragma once
#include <cstdint>
#include <vector>

namespace Mlib::ocv {

template <class TData>
class Mat;

struct KeyPoint;

//! @addtogroup features2d_main
//! @{

// //! writes vector of keypoints to the file storage
// CV_EXPORTS void write(FileStorage& fs, const String& name, const std::vector<KeyPoint>& keypoints);
// //! reads vector of keypoints from the specified file storage node
// CV_EXPORTS void read(const FileNode& node, CV_OUT std::vector<KeyPoint>& keypoints);

/** @brief A class filters a vector of keypoints.

 Because now it is difficult to provide a convenient interface for all usage scenarios of the
 keypoints filter class, it has only several needed by now static methods.
 */
class KeyPointsFilter2
{
public:
    KeyPointsFilter2(){}

    /*
     * Remove keypoints from some image by mask for pixels of this image.
     */
    static void runByPixelsMask( std::vector<KeyPoint>& keypoints, const Mat<uint8_t>& mask );
    /*
     * Remove duplicated keypoints.
     */
    static void removeDuplicatedSorted( std::vector<KeyPoint>& keypoints );

    /*
     * Retain the specified number of the best keypoints (according to the response)
     */
    static void retainBest( std::vector<KeyPoint>& keypoints, int npoints );
};

}
