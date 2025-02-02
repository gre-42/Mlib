#pragma once
#include <Mlib/Images/CvSift/CvCompat.hpp>
#include <cstdint>
#include <vector>

namespace Mlib {

template <class TData>
class Array;

}

namespace Mlib::ocv {

struct KeyPoint;

/*!
 SIFT implementation.
 
 The class implements SIFT algorithm by D. Lowe.
*/
class SIFT
{
public:
    explicit SIFT( int nfeatures=0, int nOctaveLayers=3,
          double contrastThreshold=0.04, double edgeThreshold=10,
          double sigma=1.6);

    //! returns the descriptor size in floats (128)
    int descriptorSize() const;
    
    //! finds the keypoints using SIFT algorithm
    void operator()(const Array<uint8_t>& img, const Mlib::Array<uint8_t>& mask,
                    std::vector<KeyPoint>& keypoints) const;
    //! finds the keypoints and computes descriptors for them using SIFT algorithm.
    //! Optionally it can compute descriptors for the user-provided keypoints
    void operator()(const Array<uint8_t>& img, const Array<uint8_t>& mask,
                    std::vector<KeyPoint>& keypoints,
                    Array<float>* descriptors,
                    bool useProvidedKeypoints=false) const;
    
    void buildGaussianPyramid( const Mat<int16_t>& base, std::vector<Mat<int16_t>>& pyr, int nOctaves ) const;
    void buildDoGPyramid( const std::vector<Mat<int16_t>>& pyr, std::vector<Mat<int16_t>>& dogpyr ) const;
    void findScaleSpaceExtrema( const std::vector<Mat<int16_t>>& gauss_pyr, const std::vector<Mat<int16_t>>& dog_pyr,
                                std::vector<KeyPoint>& keypoints ) const;

protected:
    int nfeatures;
    int nOctaveLayers;
    double contrastThreshold;
    double edgeThreshold;
    double sigma;
};

typedef SIFT SiftFeatureDetector;
typedef SIFT SiftDescriptorExtractor;

}
