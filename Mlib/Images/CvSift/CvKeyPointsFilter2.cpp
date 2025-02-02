/*M///////////////////////////////////////////////////////////////////////////////////////
//
//  IMPORTANT: READ BEFORE DOWNLOADING, COPYING, INSTALLING OR USING.
//
//  By downloading, copying, installing or using the software you agree to this license.
//  If you do not agree to this license, do not download, install,
//  copy or use the software.
//
//
//                           License Agreement
//                For Open Source Computer Vision Library
//
// Copyright (C) 2008, Willow Garage Inc., all rights reserved.
// Third party copyrights are property of their respective owners.
//
// Redistribution and use in source and binary forms, with or without modification,
// are permitted provided that the following conditions are met:
//
//   * Redistribution's of source code must retain the above copyright notice,
//     this list of conditions and the following disclaimer.
//
//   * Redistribution's in binary form must reproduce the above copyright notice,
//     this list of conditions and the following disclaimer in the documentation
//     and/or other materials provided with the distribution.
//
//   * The name of Intel Corporation may not be used to endorse or promote products
//     derived from this software without specific prior written permission.
//
// This software is provided by the copyright holders and contributors "as is" and
// any express or implied warranties, including, but not limited to, the implied
// warranties of merchantability and fitness for a particular purpose are disclaimed.
// In no event shall the Intel Corporation or contributors be liable for any direct,
// indirect, incidental, special, exemplary, or consequential damages
// (including, but not limited to, procurement of substitute goods or services;
// loss of use, data, or profits; or business interruption) however caused
// and on any theory of liability, whether in contract, strict liability,
// or tort (including negligence or otherwise) arising in any way out of
// the use of this software, even if advised of the possibility of such damage.
//
//M*/

#include "CvKeypointsFilter2.hpp"
#include <Mlib/Images/CvSift/KeyPoint.hpp>
#include <Mlib/Images/CvSift/CvCompat.hpp>
#include <algorithm>

namespace Mlib::ocv {

struct KeypointResponseGreaterThanOrEqualToThreshold
{
    KeypointResponseGreaterThanOrEqualToThreshold(float _value) :
    value(_value)
    {
    }
    inline bool operator()(const KeyPoint& kpt) const
    {
        return kpt.response >= value;
    }
    float value;
};

struct KeypointResponseGreater
{
    inline bool operator()(const KeyPoint& kp1, const KeyPoint& kp2) const
    {
        return kp1.response > kp2.response;
    }
};

// takes keypoints and culls them by the response
void KeyPointsFilter2::retainBest(std::vector<KeyPoint>& keypoints, int n_points)
{
    //this is only necessary if the keypoints size is greater than the number of desired points.
    if( n_points >= 0 && keypoints.size() > (size_t)n_points )
    {
        if (n_points==0)
        {
            keypoints.clear();
            return;
        }
        //first use nth element to partition the keypoints into the best and worst.
        std::nth_element(keypoints.begin(), keypoints.begin() + n_points - 1, keypoints.end(), KeypointResponseGreater());
        //this is the boundary response, and in the case of FAST may be ambiguous
        float ambiguous_response = keypoints[size_t(n_points - 1)].response;
        //use std::partition to grab all of the keypoints with the boundary response.
        std::vector<KeyPoint>::const_iterator new_end =
        std::partition(keypoints.begin() + n_points, keypoints.end(),
                       KeypointResponseGreaterThanOrEqualToThreshold(ambiguous_response));
        //resize the keypoints, given this new end point. nth_element and partition reordered the points inplace
        keypoints.resize(size_t(new_end - keypoints.begin()));
    }
}

class MaskPredicate
{
public:
    MaskPredicate( const Mat<uint8_t>& _mask ) : mask(_mask) {}
    bool operator() (const KeyPoint& key_pt) const
    {
        return mask.array( (size_t)(key_pt.pt(1) + 0.5f), (size_t)(key_pt.pt(0) + 0.5f) ) == 0;
    }

private:
    const Mat<uint8_t> mask;
    MaskPredicate& operator=(const MaskPredicate&) = delete;
};

void KeyPointsFilter2::runByPixelsMask( std::vector<KeyPoint>& keypoints, const Mat<uint8_t>& mask )
{
    if( mask.empty() )
        return;

    keypoints.erase(std::remove_if(keypoints.begin(), keypoints.end(), MaskPredicate(mask)), keypoints.end());
}

struct KeyPoint_LessThan
{
    KeyPoint_LessThan(const std::vector<KeyPoint>& _kp) : kp(&_kp) {}
    bool operator()(int i, int j) const
    {
        const KeyPoint& kp1 = (*kp)[(size_t)i];
        const KeyPoint& kp2 = (*kp)[(size_t)j];
        if( kp1.pt(0) != kp2.pt(0) )
            return kp1.pt(0) < kp2.pt(0);
        if( kp1.pt(1) != kp2.pt(1) )
            return kp1.pt(1) < kp2.pt(1);
        if( kp1.size != kp2.size )
            return kp1.size > kp2.size;
        if( kp1.angle != kp2.angle )
            return kp1.angle < kp2.angle;
        if( kp1.response != kp2.response )
            return kp1.response > kp2.response;
        if( kp1.octave != kp2.octave )
            return kp1.octave > kp2.octave;

        return i < j;
    }
    const std::vector<KeyPoint>* kp;
};

struct KeyPoint12_LessThan
{
    bool operator()(const KeyPoint &kp1, const KeyPoint &kp2) const
    {
        if( kp1.pt(0) != kp2.pt(0) )
            return kp1.pt(0) < kp2.pt(0);
        if( kp1.pt(1) != kp2.pt(1) )
            return kp1.pt(1) < kp2.pt(1);
        if( kp1.size != kp2.size )
            return kp1.size > kp2.size;
        if( kp1.angle != kp2.angle )
            return kp1.angle < kp2.angle;
        if( kp1.response != kp2.response )
            return kp1.response > kp2.response;
        return kp1.octave > kp2.octave;
    }
};

void KeyPointsFilter2::removeDuplicatedSorted( std::vector<KeyPoint>& keypoints )
{
    int i, j, n = (int)keypoints.size();

    if (n < 2) return;

    std::sort(keypoints.begin(), keypoints.end(), KeyPoint12_LessThan());

    for( i = 0, j = 1; j < n; ++j )
    {
        const KeyPoint& kp1 = keypoints[(size_t)i];
        const KeyPoint& kp2 = keypoints[(size_t)j];
        if( kp1.pt(0) != kp2.pt(0) || kp1.pt(1) != kp2.pt(1) ||
            kp1.size != kp2.size || kp1.angle != kp2.angle ) {
            keypoints[size_t(++i)] = keypoints[(size_t)j];
        }
    }
    keypoints.resize(size_t(i + 1));
}

}
