#include "CvKeyPointsFilter.hpp"
#include <Mlib/Images/CvSift/CvCompat.hpp>
#include <Mlib/Images/CvSift/KeyPoint.hpp>

using namespace Mlib::ocv;
using namespace Mlib::ocv::KeyPointsFilter;

void Mlib::ocv::KeyPointsFilter::removeDuplicated( std::vector<KeyPoint>& keypoints )
{
    int i, j, n = (int)keypoints.size();
    std::vector<int> kpidx(n);
    std::vector<uint8_t> mask(n, (uint8_t)1);

    for( i = 0; i < n; i++ )
        kpidx[i] = i;
    std::sort(kpidx.begin(), kpidx.end());
    for( i = 1, j = 0; i < n; i++ )
    {
        KeyPoint& kp1 = keypoints[kpidx[i]];
        KeyPoint& kp2 = keypoints[kpidx[j]];
        if( kp1.pt(0) != kp2.pt(0) || kp1.pt(1) != kp2.pt(1) ||
            kp1.size != kp2.size || kp1.angle != kp2.angle )
            j = i;
        else
            mask[kpidx[i]] = 0;
    }

    for( i = j = 0; i < n; i++ )
    {
        if( mask[i] )
        {
            if( i != j )
                keypoints[j] = keypoints[i];
            j++;
        }
    }
    keypoints.resize(j);
}

class MaskPredicate
{
public:
    MaskPredicate( const Mat<uint8_t>& _mask ) : mask(_mask) {}
    bool operator() (const KeyPoint& key_pt) const
    {
        return mask.array(size_t(key_pt.pt(1) + 0.5f), (size_t)(key_pt.pt(0) + 0.5f) ) == 0;
    }

private:
    const Mat<uint8_t> mask;
    MaskPredicate& operator=(const MaskPredicate&) = delete;
};

void Mlib::ocv::KeyPointsFilter::runByPixelsMask( std::vector<KeyPoint>& keypoints, const Mat<uint8_t>& mask )
{
    if( mask.empty() )
        return;

    keypoints.erase(std::remove_if(keypoints.begin(), keypoints.end(), MaskPredicate(mask)), keypoints.end());
}


struct KeypointResponseGreater
{
    inline bool operator()(const KeyPoint& kp1, const KeyPoint& kp2) const
    {
        return kp1.response > kp2.response;
    }
};

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

// takes keypoints and culls them by the response
void KeyPointsFilter::retainBest(std::vector<KeyPoint>& keypoints, int n_points)
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
        float ambiguous_response = keypoints[n_points - 1].response;
        //use std::partition to grab all of the keypoints with the boundary response.
        std::vector<KeyPoint>::const_iterator new_end =
        std::partition(keypoints.begin() + n_points, keypoints.end(),
                       KeypointResponseGreaterThanOrEqualToThreshold(ambiguous_response));
        //resize the keypoints, given this new end point. nth_element and partition reordered the points inplace
        keypoints.resize(new_end - keypoints.begin());

        // std::sort(keypoints.begin(), keypoints.end(), KeypointResponseGreater());
    }
}
