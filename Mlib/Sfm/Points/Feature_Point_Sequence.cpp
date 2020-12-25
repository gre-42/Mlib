#include "Feature_Point_Sequence.hpp"

using namespace Mlib::Sfm;

std::shared_ptr<FeaturePoint> FeaturePointSequence::nth_from_end(size_t n)
{
    auto it = sequence.crbegin();
    for (; (it != sequence.crend()) && (n-- != 0); ++it);
    if (it == sequence.crend()) {
        return nullptr;
    } else {
        return it->second;
    }
}
