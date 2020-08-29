#pragma once
#include <Mlib/Array/Array.hpp>

namespace Mlib { namespace Sfm {

class NormalizedProjection {
public:
    NormalizedProjection(const Array<float>& p);
    Array<float> normalized_y(const Array<float>& y);
    Array<float> denormalized_intrinsic_matrix(const Array<float>& m);
    Array<float> normalized_intrinsic_matrix(const Array<float>& m);
    void print_min_max() const;
    float min_x = std::numeric_limits<float>::infinity();
    float max_x = -std::numeric_limits<float>::infinity();
    float min_y = std::numeric_limits<float>::infinity();
    float max_y = -std::numeric_limits<float>::infinity();
    Array<float> yn;
    Array<float> N;
};

}}
