#include "Depth_Confidence.hpp"
#include <Mlib/Stats/Min_Max.hpp>

using namespace Mlib;
using namespace Mlib::Sfm;

Array<float> Mlib::Sfm::depth_confidence(const Array<float>& cost_volume) {
    assert(cost_volume.ndim() == 3);

    Array<float> min_cost = min(substitute_nans(cost_volume, INFINITY), 0);
    Array<float> result = zeros<float>(cost_volume.shape().erased_first());
    Array<size_t> nelements = zeros<size_t>(result.shape());
    float isigma2 = 1.f / squared(0.1f);
    for (size_t h = 0; h < cost_volume.shape(0); ++h) {
        for (size_t r = 0; r < cost_volume.shape(1); ++r) {
            for (size_t c = 0; c < cost_volume.shape(2); ++c) {
                if (!std::isnan(cost_volume(h, r, c))) {
                    result(r, c) += std::exp(-squared(cost_volume(h, r, c) - min_cost(r, c)) * isigma2);
                    ++nelements(r, c);
                }
            }
        }
    }
    return result.array_array_binop(nelements, [&cost_volume](float r, size_t n){
        return n == 0
            ? NAN
            : cost_volume.shape(0) * n / r;});
}
