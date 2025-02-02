#pragma once
#include <memory>
#include <vector>

namespace Mlib {

template <class TData>
class Array;

}

namespace Mlib::ocv {

template <class TData>
class Mat;

struct KeyPoint;

/*!
 SIFT implementation.

 The class implements SIFT algorithm by D. Lowe.
 */
class SIFT2
{
public:
    explicit SIFT2( int nfeatures = 0, int nOctaveLayers = 3,
                    double contrastThreshold = 0.04, double edgeThreshold = 10,
                    double sigma = 1.6 );

    //! returns the descriptor size in floats (128)
    int descriptorSize() const;

    //! finds the keypoints and computes descriptors for them using SIFT algorithm.
    //! Optionally it can compute descriptors for the user-provided keypoints
    void detectAndCompute(const Array<uint8_t>& img, const Array<uint8_t>& mask,
                    std::vector<KeyPoint>& keypoints,
                    Array<float>* descriptors,
                    bool useProvidedKeypoints = false);

    void buildGaussianPyramid( const Mat<float>& base, std::vector<Mat<float>>& pyr, int nOctaves ) const;
    void buildDoGPyramid( const std::vector<Mat<float>>& pyr, std::vector<Mat<float>>& dogpyr ) const;
    void findScaleSpaceExtrema( const std::vector<Mat<float>>& gauss_pyr, const std::vector<Mat<float>>& dog_pyr,
                               std::vector<KeyPoint>& keypoints ) const;

protected:
    int nfeatures;
    int nOctaveLayers;
    double contrastThreshold;
    double edgeThreshold;
    double sigma;
};

}
