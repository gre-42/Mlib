/*M///////////////////////////////////////////////////////////////////////////////////////
//
//  IMPORTANT: READ BEFORE DOWNLOADING, COPYING, INSTALLING OR USING.
//
//  By downloading, copying, installing or using the software you agree to this license.
//  If you do not agree to this license, do not download, install,
//  copy or use the software.
//
//
//                          License Agreement
//                For Open Source Computer Vision Library
//
// Copyright (C) 2000-2008, Intel Corporation, all rights reserved.
// Copyright (C) 2009, Willow Garage Inc., all rights reserved.
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
//   * The name of the copyright holders may not be used to endorse or promote products
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

/**********************************************************************************************\
 Implementation of SIFT is based on the code from http://blogs.oregonstate.edu/hess/code/sift/
 Below is the original copyright.

//    Copyright (c) 2006-2010, Rob Hess <hess@eecs.oregonstate.edu>
//    All rights reserved.

//    The following patent has been issued for methods embodied in this
//    software: "Method and apparatus for identifying scale invariant features
//    in an image and use of same for locating an object in an image," David
//    G. Lowe, US Patent 6,711,293 (March 23, 2004). Provisional application
//    filed March 8, 1999. Asignee: The University of British Columbia. For
//    further details, contact David Lowe (lowe@cs.ubc.ca) or the
//    University-Industry Liaison Office of the University of British
//    Columbia.

//    Note that restrictions imposed by this patent (and possibly others)
//    exist independently of and may be in conflict with the freedoms granted
//    in this license, which refers to copyright of the program, not patents
//    for any methods that it implements.  Both copyright and patent law must
//    be obeyed to legally use and redistribute this program and it is not the
//    purpose of this license to induce you to infringe any patents or other
//    property right claims or to contest validity of any such claims.  If you
//    redistribute or use the program, then this license merely protects you
//    from committing copyright infringement.  It does not protect you from
//    committing patent infringement.  So, before you do anything with this
//    program, make sure that you have permission to do so not merely in terms
//    of copyright, but also in terms of patent law.

//    Please note that this license is not to be understood as a guarantee
//    either.  If you use the program according to this license, but in
//    conflict with patent law, it does not mean that the licensor will refund
//    you for any losses that you incur if you are sued for your patent
//    infringement.

//    Redistribution and use in source and binary forms, with or without
//    modification, are permitted provided that the following conditions are
//    met:
//        * Redistributions of source code must retain the above copyright and
//          patent notices, this list of conditions and the following
//          disclaimer.
//        * Redistributions in binary form must reproduce the above copyright
//          notice, this list of conditions and the following disclaimer in
//          the documentation and/or other materials provided with the
//          distribution.
//        * Neither the name of Oregon State University nor the names of its
//          contributors may be used to endorse or promote products derived
//          from this software without specific prior written permission.

//    THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS
//    IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
//    TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
//    PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
//    HOLDER BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
//    EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
//    PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
//    PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
//    LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
//    NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
//    SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
\**********************************************************************************************/

#include <Mlib/Images/CvSift/CvSift.hpp>
#include <Mlib/Images/CvSift/CvCompat.hpp>
#include <Mlib/Images/CvSift/CvKeyPointsFilter.hpp>
#include <Mlib/Images/CvSift/KeyPoint.hpp>
#include <Mlib/Images/Resample/Pyramid.hpp>
#include <Mlib/Floating_Point_Exceptions.hpp>
#include <Mlib/Math/Fixed_Cholesky.hpp>
#include <cfloat>
#include <iostream>
#include <stdarg.h>

using namespace Mlib;
using namespace Mlib::ocv;

/******************************* Defs and macros *****************************/

// // default number of sampled intervals per octave
// static const int SIFT_INTVLS = 3;
// 
// // default sigma for initial gaussian smoothing
// static const float SIFT_SIGMA = 1.6f;
// 
// // default threshold on keypoint contrast |D(x)|
// static const float SIFT_CONTR_THR = 0.04f;
// 
// // default threshold on keypoint ratio of principle curvatures
// static const float SIFT_CURV_THR = 10.f;
// 
// // double image size before pyramid construction?
// static const bool SIFT_IMG_DBL = true;

// default width of descriptor histogram array
static const int SIFT_DESCR_WIDTH = 4;

// default number of bins per histogram in descriptor array
static const int SIFT_DESCR_HIST_BINS = 8;

// assumed gaussian blur for input image
static const float SIFT_INIT_SIGMA = 0.5f;

// width of border in which to ignore keypoints
static const int SIFT_IMG_BORDER = 5;

// maximum steps of keypoint interpolation before failure
static const int SIFT_MAX_INTERP_STEPS = 5;

// default number of bins in histogram for orientation assignment
static const int SIFT_ORI_HIST_BINS = 36;

// determines gaussian sigma for orientation assignment
static const float SIFT_ORI_SIG_FCTR = 1.5f;

// determines the radius of the region used in orientation assignment
static const float SIFT_ORI_RADIUS = 3 * SIFT_ORI_SIG_FCTR;

// orientation magnitude relative to max that results in new feature
static const float SIFT_ORI_PEAK_RATIO = 0.8f;

// determines the size of a single descriptor orientation histogram
static const float SIFT_DESCR_SCL_FCTR = 3.f;

// threshold on magnitude of elements of descriptor vector
static const float SIFT_DESCR_MAG_THR = 0.2f;

// factor used to convert floating-point descriptor to unsigned char
static const float SIFT_INT_DESCR_FCTR = 512.f;

static const int SIFT_FIXPT_SCALE = 48;

#define CV_PI   3.1415926535897932384626433832795

static Mat<int16_t> createInitialImage( const Mat<uint8_t>& img, bool doubleImageSize, float sigma )
{
    Mat<uint8_t> gray;
    Mat<int16_t> gray_fpt;
    if( img.channels() == 3 || img.channels() == 4 )
        cvtColor(img, gray, COLOR_BGR2GRAY);
    else
        img.copyTo(gray);
    gray.convertTo(gray_fpt, SIFT_FIXPT_SCALE);

    float sig_diff;

    if( doubleImageSize )
    {
        sig_diff = sqrtf( std::max(sigma * sigma - SIFT_INIT_SIGMA * SIFT_INIT_SIGMA * 4, 0.01f) );
        Mat<int16_t> dbl;
        dbl.array = Mlib::up_sample2(gray_fpt.array);
        GaussianBlur(dbl, dbl, sig_diff);
        return dbl;
    }
    else
    {
        sig_diff = sqrtf( std::max(sigma * sigma - SIFT_INIT_SIGMA * SIFT_INIT_SIGMA, 0.01f) );
        GaussianBlur(gray_fpt, gray_fpt, sig_diff);
        return gray_fpt;
    }
}


void SIFT::buildGaussianPyramid( const Mat<int16_t>& base, std::vector<Mat<int16_t>>& pyr, int nOctaves ) const
{
    std::vector<double> sig(size_t(nOctaveLayers + 3));
    pyr.resize(size_t(nOctaves*(nOctaveLayers + 3)));

    // precompute Gaussian sigmas using the following formula:
    //  \sigma_{total}^2 = \sigma_{i}^2 + \sigma_{i-1}^2
    sig[0] = sigma;
    double k = std::pow( 2., 1. / nOctaveLayers );
    for( int i = 1; i < nOctaveLayers + 3; i++ )
    {
        double sig_prev = std::pow(k, (double)(i-1))*sigma;
        double sig_total = sig_prev*k;
        sig[(size_t)i] = std::sqrt(sig_total*sig_total - sig_prev*sig_prev);
    }

    for( int o = 0; o < nOctaves; o++ )
    {
        for( int i = 0; i < nOctaveLayers + 3; i++ )
        {
            Mat<int16_t>& dst = pyr[size_t(o*(nOctaveLayers + 3) + i)];
            assert(dst.empty());
            if( o == 0  &&  i == 0 )
                dst.array = base.array;
            // base of new octave is halved image from end of previous octave
            else if( i == 0 )
            {
                const Mat<int16_t>& src = pyr[size_t((o-1)*(nOctaveLayers + 3) + nOctaveLayers)];
                dst.array = down_sample2(src.array);
            }
            else
            {
                const Mat<int16_t>& src = pyr[size_t(o*(nOctaveLayers + 3) + i-1)];
                GaussianBlur(src, dst, (float)sig[(size_t)i]);
            }
        }
    }
}


void SIFT::buildDoGPyramid( const std::vector<Mat<int16_t>>& gpyr, std::vector<Mat<int16_t>>& dogpyr ) const
{
    int nOctaves = (int)gpyr.size()/(nOctaveLayers + 3);
    dogpyr.resize( size_t(nOctaves*(nOctaveLayers + 2)) );

    for( int o = 0; o < nOctaves; o++ )
    {
        for( int i = 0; i < nOctaveLayers + 2; i++ )
        {
            const Mat<int16_t>& src1 = gpyr[size_t(o*(nOctaveLayers + 3) + i)];
            const Mat<int16_t>& src2 = gpyr[size_t(o*(nOctaveLayers + 3) + i + 1)];
            Mat<int16_t>& dst = dogpyr[size_t(o*(nOctaveLayers + 2) + i)];
            dst.array.move() = src2.array - src1.array;
        }
    }
}


// Computes a gradient orientation histogram at a specified pixel
static float calcOrientationHist( const Mat<int16_t>& img, const FixedArray<int, 2>& pt, int radius,
                                  float sigma, float* hist, int n )
{
    int i, j, k, len = (radius*2+1)*(radius*2+1);

    float expf_scale = -1.f/(2.f * sigma * sigma);
    std::vector<float> buf(size_t(len*4 + n+4));
    float *X = buf.data(), *Y = X + len, *Mag = X, *Ori = Y + len, *W = Ori + len;
    float* temphist = W + len + 2;

    for( i = 0; i < n; i++ )
        temphist[i] = 0.f;

    for( i = -radius, k = 0; i <= radius; i++ )
    {
        int y = pt(1) + i;
        if( y <= 0 || y >= img.rows() - 1 )
            continue;
        for( j = -radius; j <= radius; j++ )
        {
            int x = pt(0) + j;
            if( x <= 0 || x >= img.cols() - 1 )
                continue;

            float dx = (float)(img.array((size_t)y, (size_t)x+1) - img.array((size_t)y, (size_t)x-1));
            float dy = (float)(img.array((size_t)y-1, (size_t)x) - img.array((size_t)y+1, (size_t)x));

            X[(size_t)k] = dx; Y[(size_t)k] = dy; W[(size_t)k] = float(i*i + j*j)*expf_scale;
            k++;
        }
    }

    len = k;

    // compute gradient values, orientations and the weights over the pixel neighborhood
    exp(W, W, len);
    fastAtan2(Y, X, Ori, len);
    magnitude(X, Y, Mag, len);

    for( k = 0; k < len; k++ )
    {
        int bin = cvRound(((float)n/360.f)*Ori[k]);
        if( bin >= n )
            bin -= n;
        if( bin < 0 )
            bin += n;
        temphist[bin] += W[k]*Mag[k];
    }

    // smooth the histogram
    temphist[-1] = temphist[n-1];
    temphist[-2] = temphist[n-2];
    temphist[n] = temphist[0];
    temphist[n+1] = temphist[1];
    for( i = 0; i < n; i++ )
    {
        hist[i] = (temphist[i-2] + temphist[i+2])*(1.f/16.f) +
            (temphist[i-1] + temphist[i+1])*(4.f/16.f) +
            temphist[i]*(6.f/16.f);
    }

    float maxval = hist[0];
    for( i = 1; i < n; i++ )
        maxval = std::max(maxval, hist[i]);

    return maxval;
}


//
// Interpolates a scale-space extremum's location and scale to subpixel
// accuracy to form an image feature.  Rejects features with low contrast.
// Based on Section 4 of Lowe's paper.
static bool adjustLocalExtrema( const std::vector<Mat<int16_t>>& dog_pyr, KeyPoint& kpt, int octv,
                                int& layer, int& r, int& c, int nOctaveLayers,
                                float contrastThreshold, float edgeThreshold, float sigma )
{
    const float img_scale = 1.f/(255*SIFT_FIXPT_SCALE);
    const float deriv_scale = img_scale*0.5f;
    const float second_deriv_scale = img_scale;
    const float cross_deriv_scale = img_scale*0.25f;

    float xi=0, xr=0, xc=0, contr=0;
    int i = 0;

    TemporarilyIgnoreFloatingPointExeptions ignore_except;
    for( ; i < SIFT_MAX_INTERP_STEPS; i++ )
    {
        int idx = octv*(nOctaveLayers+2) + layer;
        const Mat<int16_t>& img = dog_pyr[(size_t)idx];
        const Mat<int16_t>& prev = dog_pyr[(size_t)idx-1];
        const Mat<int16_t>& next = dog_pyr[(size_t)idx+1];

        FixedArray<float, 3> dD{
            (img.array((size_t)r, (size_t)c+1) - img.array((size_t)r, (size_t)c-1))*deriv_scale,
            (img.array((size_t)r+1, (size_t)c) - img.array((size_t)r-1, (size_t)c))*deriv_scale,
            (next.array((size_t)r, (size_t)c) - prev.array((size_t)r, (size_t)c))*deriv_scale};

        float v2 = (float)img.array((size_t)r, (size_t)c)*2;
        float dxx = (img.array((size_t)r, (size_t)c+1) + img.array((size_t)r, (size_t)c-1) - v2)*second_deriv_scale;
        float dyy = (img.array((size_t)r+1, (size_t)c) + img.array((size_t)r-1, (size_t)c) - v2)*second_deriv_scale;
        float dss = (next.array((size_t)r, (size_t)c) + prev.array((size_t)r, (size_t)c) - v2)*second_deriv_scale;
        float dxy = (img.array((size_t)r+1, (size_t)c+1) - img.array((size_t)r+1, (size_t)c-1) -
                     img.array((size_t)r-1, (size_t)c+1) + img.array((size_t)r-1, (size_t)c-1))*cross_deriv_scale;
        float dxs = (next.array((size_t)r, (size_t)c+1) - next.array((size_t)r, (size_t)c-1) -
                     prev.array((size_t)r, (size_t)c+1) + prev.array((size_t)r, (size_t)c-1))*cross_deriv_scale;
        float dys = (next.array((size_t)r+1, (size_t)c) - next.array((size_t)r-1, (size_t)c) -
                     prev.array((size_t)r+1, (size_t)c) + prev.array((size_t)r-1, (size_t)c))*cross_deriv_scale;

        auto H = FixedArray<float, 3, 3>::init(
            dxx, dxy, dxs,
            dxy, dyy, dys,
            dxs, dys, dss);

        FixedArray<float, 3> X = lstsq_chol_1d(H, dD).value();
        if (any(Mlib::isnan(X))) {
            return false;
        }

        xi = -X(2);
        xr = -X(1);
        xc = -X(0);

        if( std::abs(xi) < 0.5f && std::abs(xr) < 0.5f && std::abs(xc) < 0.5f )
            break;

        c += cvRound(xc);
        r += cvRound(xr);
        layer += cvRound(xi);

        if( layer < 1 || layer > nOctaveLayers ||
           c < SIFT_IMG_BORDER || c >= img.cols() - SIFT_IMG_BORDER  ||
           r < SIFT_IMG_BORDER || r >= img.rows() - SIFT_IMG_BORDER )
            return false;
    }

    // ensure convergence of interpolation
    if( i >= SIFT_MAX_INTERP_STEPS )
        return false;

    {
        int idx = octv*(nOctaveLayers+2) + layer;
        const Mat<int16_t>& img = dog_pyr[(size_t)idx];
        const Mat<int16_t>& prev = dog_pyr[(size_t)idx-1];
        const Mat<int16_t>& next = dog_pyr[(size_t)idx+1];
        FixedArray<float, 3> dD{
            (img.array((size_t)r, (size_t)c+1) - img.array((size_t)r, (size_t)c-1))*deriv_scale,
            (img.array((size_t)r+1, (size_t)c) - img.array((size_t)r-1, (size_t)c))*deriv_scale,
            (next.array((size_t)r, (size_t)c) - prev.array((size_t)r, (size_t)c))*deriv_scale};
        float t = dot0d(dD, FixedArray<float, 3>{xc, xr, xi});

        contr = img.array((size_t)r, (size_t)c)*img_scale + t * 0.5f;
        if( std::abs( contr ) * (float)nOctaveLayers < contrastThreshold )
            return false;

        // principal curvatures are computed using the trace and det of Hessian
        float v2 = img.array((size_t)r, (size_t)c)*2.f;
        float dxx = (img.array((size_t)r, (size_t)c+1) + img.array((size_t)r, (size_t)c-1) - v2)*second_deriv_scale;
        float dyy = (img.array((size_t)r+1, (size_t)c) + img.array((size_t)r-1, (size_t)c) - v2)*second_deriv_scale;
        float dxy = (img.array((size_t)r+1, (size_t)c+1) - img.array((size_t)r+1, (size_t)c-1) -
                     img.array((size_t)r-1, (size_t)c+1) + img.array((size_t)r-1, (size_t)c-1)) * cross_deriv_scale;
        float tr = dxx + dyy;
        float det = dxx * dyy - dxy * dxy;

        if( det <= 0 || tr*tr*edgeThreshold >= (edgeThreshold + 1)*(edgeThreshold + 1)*det )
            return false;
    }

    kpt.pt(0) = ((float)c + xc) * float(1 << octv);
    kpt.pt(1) = ((float)r + xr) * float(1 << octv);
    kpt.octave = octv + (layer << 8) + (cvRound((xi + 0.5)*255) << 16);
    kpt.size = sigma*powf(2.f, ((float)layer + xi) / (float)nOctaveLayers)*float(1 << octv)*2.f;
    kpt.response = std::abs(contr);

    return true;
}


//
// Detects features at extrema in DoG scale space.  Bad features are discarded
// based on contrast and ratio of principal curvatures.
void SIFT::findScaleSpaceExtrema( const std::vector<Mat<int16_t>>& gauss_pyr, const std::vector<Mat<int16_t>>& dog_pyr,
                                  std::vector<KeyPoint>& keypoints ) const
{
    int nOctaves = (int)gauss_pyr.size()/(nOctaveLayers + 3);
    int threshold = cvFloor(0.5f * (float)contrastThreshold / (float)nOctaveLayers * 255.f * (float)SIFT_FIXPT_SCALE);
    const int n = SIFT_ORI_HIST_BINS;
    float hist[n];
    KeyPoint kpt;

    keypoints.clear();

    for( int o = 0; o < nOctaves; o++ )
        for( int i = 1; i <= nOctaveLayers; i++ )
        {
            int idx = o*(nOctaveLayers+2)+i;
            const Mat<int16_t>& img = dog_pyr[(size_t)idx];
            const Mat<int16_t>& prev = dog_pyr[(size_t)idx-1];
            const Mat<int16_t>& next = dog_pyr[(size_t)idx+1];
            int step = (int)img.step1();
            int rows = img.rows(), cols = img.cols();

            for( int r = SIFT_IMG_BORDER; r < rows-SIFT_IMG_BORDER; r++)
            {
                const int16_t* currptr = img.ptr(r);
                const int16_t* prevptr = prev.ptr(r);
                const int16_t* nextptr = next.ptr(r);

                for( int c = SIFT_IMG_BORDER; c < cols-SIFT_IMG_BORDER; c++)
                {
                    int val = currptr[c];

                    // find local extrema with pixel accuracy
                    if( std::abs(val) > threshold &&
                       ((val > 0 && val >= currptr[c-1] && val >= currptr[c+1] &&
                         val >= currptr[c-step-1] && val >= currptr[c-step] && val >= currptr[c-step+1] &&
                         val >= currptr[c+step-1] && val >= currptr[c+step] && val >= currptr[c+step+1] &&
                         val >= nextptr[c] && val >= nextptr[c-1] && val >= nextptr[c+1] &&
                         val >= nextptr[c-step-1] && val >= nextptr[c-step] && val >= nextptr[c-step+1] &&
                         val >= nextptr[c+step-1] && val >= nextptr[c+step] && val >= nextptr[c+step+1] &&
                         val >= prevptr[c] && val >= prevptr[c-1] && val >= prevptr[c+1] &&
                         val >= prevptr[c-step-1] && val >= prevptr[c-step] && val >= prevptr[c-step+1] &&
                         val >= prevptr[c+step-1] && val >= prevptr[c+step] && val >= prevptr[c+step+1]) ||
                        (val < 0 && val <= currptr[c-1] && val <= currptr[c+1] &&
                         val <= currptr[c-step-1] && val <= currptr[c-step] && val <= currptr[c-step+1] &&
                         val <= currptr[c+step-1] && val <= currptr[c+step] && val <= currptr[c+step+1] &&
                         val <= nextptr[c] && val <= nextptr[c-1] && val <= nextptr[c+1] &&
                         val <= nextptr[c-step-1] && val <= nextptr[c-step] && val <= nextptr[c-step+1] &&
                         val <= nextptr[c+step-1] && val <= nextptr[c+step] && val <= nextptr[c+step+1] &&
                         val <= prevptr[c] && val <= prevptr[c-1] && val <= prevptr[c+1] &&
                         val <= prevptr[c-step-1] && val <= prevptr[c-step] && val <= prevptr[c-step+1] &&
                         val <= prevptr[c+step-1] && val <= prevptr[c+step] && val <= prevptr[c+step+1])))
                    {
                        int r1 = r, c1 = c, layer = i;
                        if( !adjustLocalExtrema(dog_pyr, kpt, o, layer, r1, c1,
                                                nOctaveLayers, (float)contrastThreshold,
                                                (float)edgeThreshold, (float)sigma) )
                            continue;
                        float scl_octv = kpt.size*0.5f/float(1 << o);
                        float omax = calcOrientationHist(gauss_pyr[size_t(o*(nOctaveLayers+3) + layer)],
                                                         FixedArray<int, 2>{c1, r1},
                                                         cvRound(SIFT_ORI_RADIUS * scl_octv),
                                                         SIFT_ORI_SIG_FCTR * scl_octv,
                                                         hist, n);
                        float mag_thr = (float)(omax * SIFT_ORI_PEAK_RATIO);
                        for( int j = 0; j < n; j++ )
                        {
                            int l = j > 0 ? j - 1 : n - 1;
                            int r2 = j < n-1 ? j + 1 : 0;

                            if( hist[j] > hist[l]  &&  hist[j] > hist[r2]  &&  hist[j] >= mag_thr )
                            {
                                float bin = (float)j + 0.5f * (hist[l]-hist[r2]) / (hist[l] - 2.f*hist[j] + hist[r2]);
                                bin = bin < 0 ? n + bin : bin >= n ? bin - n : bin;
                                kpt.angle = 360.f - (float)((360.f/n) * bin);
                                if(std::abs(kpt.angle - 360.f) < FLT_EPSILON)
                                       kpt.angle = 0.f;
                                keypoints.push_back(kpt);
                            }
                        }
                    }
                }
            }
        }
}


static void calcSIFTDescriptor( const Mat<int16_t>& img, const FixedArray<float, 2>& ptf, float ori, float scl,
                               int d, int n, float* dst )
{
    FixedArray<int, 2> pt(cvRound(ptf(0)), cvRound(ptf(1)));
    float cos_t = cosf(ori*(float)(CV_PI/180));
    float sin_t = sinf(ori*(float)(CV_PI/180));
    float bins_per_rad = (float)n / 360.f;
    float exp_scale = -1.f/(float(d * d) * 0.5f);
    float hist_width = SIFT_DESCR_SCL_FCTR * scl;
    int radius = cvRound(hist_width * 1.4142135623730951f * float(d + 1) * 0.5f);
    cos_t /= hist_width;
    sin_t /= hist_width;

    int i, j, k, len = (radius*2+1)*(radius*2+1), histlen = (d+2)*(d+2)*(n+2);
    int rows = img.rows(), cols = img.cols();

    std::vector<float> buf(size_t(len*6 + histlen));
    float *X = buf.data(), *Y = X + len, *Mag = Y, *Ori = Mag + len, *W = Ori + len;
    float *RBin = W + len, *CBin = RBin + len, *hist = CBin + len;

    for( i = 0; i < d+2; i++ )
    {
        for( j = 0; j < d+2; j++ )
            for( k = 0; k < n+2; k++ )
                hist[(i*(d+2) + j)*(n+2) + k] = 0.;
    }

    for( i = -radius, k = 0; i <= radius; i++ )
        for( j = -radius; j <= radius; j++ )
        {
            // Calculate sample's histogram array coords rotated relative to ori.
            // Subtract 0.5 so samples that fall e.g. in the center of row 1 (i.e.
            // r_rot = 1.5) have full weight placed in row 1 after interpolation.
            float c_rot = (float)j * cos_t - (float)i * sin_t;
            float r_rot = (float)j * sin_t + (float)i * cos_t;
            float rbin = r_rot + float(d/2) - 0.5f;
            float cbin = c_rot + float(d/2) - 0.5f;
            int r = pt(1) + i, c = pt(0) + j;

            if( rbin > -1 && rbin < (float)d && cbin > -1 && cbin < (float)d &&
               r > 0 && r < rows - 1 && c > 0 && c < cols - 1 )
            {
                float dx = (float)(img.array((size_t)r, (size_t)c+1) - img.array((size_t)r, (size_t)c-1));
                float dy = (float)(img.array((size_t)r-1, (size_t)c) - img.array((size_t)r+1, (size_t)c));
                X[k] = dx; Y[k] = dy; RBin[k] = rbin; CBin[k] = cbin;
                W[k] = (c_rot * c_rot + r_rot * r_rot)*exp_scale;
                k++;
            }
        }

    len = k;
    fastAtan2(Y, X, Ori, len);
    magnitude(X, Y, Mag, len);
    exp(W, W, len);

    for( k = 0; k < len; k++ )
    {
        float rbin = RBin[k], cbin = CBin[k];
        float obin = (Ori[k] - ori)*bins_per_rad;
        float mag = Mag[k]*W[k];

        int r0 = cvFloor( rbin );
        int c0 = cvFloor( cbin );
        int o0 = cvFloor( obin );
        rbin -= (float)r0;
        cbin -= (float)c0;
        obin -= (float)o0;

        if( o0 < 0 )
            o0 += n;
        if( o0 >= n )
            o0 -= n;

        // histogram update using tri-linear interpolation
        float v_r1 = mag*rbin, v_r0 = mag - v_r1;
        float v_rc11 = v_r1*cbin, v_rc10 = v_r1 - v_rc11;
        float v_rc01 = v_r0*cbin, v_rc00 = v_r0 - v_rc01;
        float v_rco111 = v_rc11*obin, v_rco110 = v_rc11 - v_rco111;
        float v_rco101 = v_rc10*obin, v_rco100 = v_rc10 - v_rco101;
        float v_rco011 = v_rc01*obin, v_rco010 = v_rc01 - v_rco011;
        float v_rco001 = v_rc00*obin, v_rco000 = v_rc00 - v_rco001;

        int idx = ((r0+1)*(d+2) + c0+1)*(n+2) + o0;
        hist[idx] += v_rco000;
        hist[idx+1] += v_rco001;
        hist[idx+(n+2)] += v_rco010;
        hist[idx+(n+3)] += v_rco011;
        hist[idx+(d+2)*(n+2)] += v_rco100;
        hist[idx+(d+2)*(n+2)+1] += v_rco101;
        hist[idx+(d+3)*(n+2)] += v_rco110;
        hist[idx+(d+3)*(n+2)+1] += v_rco111;
    }

    // finalize histogram, since the orientation histograms are circular
    for( i = 0; i < d; i++ )
        for( j = 0; j < d; j++ )
        {
            int idx = ((i+1)*(d+2) + (j+1))*(n+2);
            hist[idx] += hist[idx+n];
            hist[idx+1] += hist[idx+n+1];
            for( k = 0; k < n; k++ )
                dst[(i*d + j)*n + k] = hist[idx+k];
        }
    // copy histogram to the descriptor,
    // apply hysteresis thresholding
    // and scale the result, so that it can be easily converted
    // to byte array
    float nrm2 = 0;
    len = d*d*n;
    for( k = 0; k < len; k++ )
        nrm2 += dst[k]*dst[k];
    float thr = std::sqrt(nrm2)*SIFT_DESCR_MAG_THR;
    for( i = 0, nrm2 = 0; i < k; i++ )
    {
        float val = std::min(dst[i], thr);
        dst[i] = val;
        nrm2 += val*val;
    }
    nrm2 = SIFT_INT_DESCR_FCTR/std::max(std::sqrt(nrm2), FLT_EPSILON);
    for( k = 0; k < len; k++ )
    {
        dst[k] = saturate_cast<uint8_t>(dst[k]*nrm2);
    }
}

static void calcDescriptors(const std::vector<Mat<int16_t>>& gpyr, const std::vector<KeyPoint>& keypoints,
                            Mat<float>& descriptors, int nOctaveLayers )
{
    int d = SIFT_DESCR_WIDTH, n = SIFT_DESCR_HIST_BINS;

    for( size_t i = 0; i < keypoints.size(); i++ )
    {
        KeyPoint kpt = keypoints[i];
        int octv=kpt.octave & 255, layer=(kpt.octave >> 8) & 255;
        float scale = 1.f/float(1 << octv);
        float size=kpt.size*scale;
        FixedArray<float, 2> ptf(kpt.pt(0)*scale, kpt.pt(1)*scale);
        const Mat<int16_t>& img = gpyr[size_t(octv*(nOctaveLayers + 3) + layer)];

        float angle = 360.f - kpt.angle;
        if(std::abs(angle - 360.f) < FLT_EPSILON)
           angle = 0.f;
        calcSIFTDescriptor(img, ptf, angle, size*0.5f, d, n, descriptors.ptr((int)i));
    }
}

//////////////////////////////////////////////////////////////////////////////////////////

SIFT::SIFT( int _nfeatures, int _nOctaveLayers,
           double _contrastThreshold, double _edgeThreshold, double _sigma )
    : nfeatures(_nfeatures), nOctaveLayers(_nOctaveLayers),
    contrastThreshold(_contrastThreshold), edgeThreshold(_edgeThreshold), sigma(_sigma)
{
}

int SIFT::descriptorSize() const
{
    return SIFT_DESCR_WIDTH*SIFT_DESCR_WIDTH*SIFT_DESCR_HIST_BINS;
}

void SIFT::operator()(const Array<uint8_t>& image, const Array<uint8_t>& mask,
                      std::vector<KeyPoint>& keypoints) const
{
    (*this)(image, mask, keypoints, nullptr);
}


void SIFT::operator()(const Array<uint8_t>& _image, const Array<uint8_t>& _mask,
                      std::vector<KeyPoint>& keypoints,
                      Array<float>* _descriptors,
                      bool useProvidedKeypoints) const
{
    Mat<uint8_t> image{ _image };
    Mat<uint8_t> mask{ _mask };

    Mat<int16_t> base = createInitialImage(image, false, (float)sigma);
    std::vector<Mat<int16_t>> gpyr, dogpyr;
    int nOctaves = cvRound(std::log( (double)std::min( base.cols(), base.rows() ) ) / std::log(2.) - 2);

    //double t, tf = getTickFrequency();
    //t = (double)getTickCount();
    buildGaussianPyramid(base, gpyr, nOctaves);
    buildDoGPyramid(gpyr, dogpyr);
    // for (size_t i = 0; i < gpyr.size(); ++i) {
    //     draw_nan_masked_grayscale(gpyr[i].array.casted<float>(), 0.f, float{INT16_MAX}).save_to_file(std::to_string(i) + "_g.png");
    // }
    // for (size_t i = 0; i < dogpyr.size(); ++i) {
    //     draw_nan_masked_grayscale(dogpyr[i].array.casted<float>(), float{100}, float{100}).save_to_file(std::to_string(i) + "_d.png");
    // }

    //t = (double)getTickCount() - t;
    //printf("pyramid construction time: %g\n", t*1000./tf);

    if( !useProvidedKeypoints )
    {
        //t = (double)getTickCount();
        findScaleSpaceExtrema(gpyr, dogpyr, keypoints);
        KeyPointsFilter::removeDuplicated( keypoints );

        if( !mask.empty() )
            KeyPointsFilter::runByPixelsMask( keypoints, mask );

        if( nfeatures > 0 )
            KeyPointsFilter::retainBest(keypoints, nfeatures);
        //t = (double)getTickCount() - t;
        //printf("keypoint detection time: %g\n", t*1000./tf);
    }
    else
    {
        // filter keypoints by mask
        //KeyPointsFilter::runByPixelsMask( keypoints, mask );
    }

    if( _descriptors != nullptr )
    {
        //t = (double)getTickCount();
        int dsize = descriptorSize();
        _descriptors->resize(ArrayShape{ (size_t)keypoints.size(), (size_t)dsize });
        Mat<float> descriptors{ *_descriptors };

        calcDescriptors(gpyr, keypoints, descriptors, nOctaveLayers);
        //t = (double)getTickCount() - t;
        //printf("descriptor extraction time: %g\n", t*1000./tf);
    }
}
