#pragma once
#include <Mlib/Array/Array_Forward.hpp>
#include <Mlib/Stats/Histogram_Matching.hpp>

namespace Mlib {

template <class TData, class TDataRef, class TFloat>
struct HistogramsAndMatched {
    std::vector<HistogramMatching<TData, TDataRef, TFloat>> hms;
    Array<TDataRef> matched;
};

HistogramsAndMatched<unsigned char, unsigned char, float> match_rgba_histograms(
    const Array<unsigned char>& image,
    const Array<unsigned char>& ref,
    unsigned char alpha_threshold = 20);

HistogramsAndMatched<float, float, float> match_rgba_histograms(
    const Array<float>& image,
    const Array<float>& ref,
    float alpha_threshold = 0.05f);

}
