#pragma once
#include <Mlib/Array/Array.hpp>

namespace Mlib { namespace Sfm {

class SyntheticScene {
public:
    SyntheticScene(
        bool zero_first_extrinsic = false,
        float tR_multiplier = 1);
    Array<float> delta_ke(size_t index0, size_t index1);
    Array<float> dt2(size_t index0, size_t index1);
    Array<float> dt(size_t index0, size_t index1);
    Array<float> dR(size_t index0, size_t index1);
    void draw_to_bmp(const std::string& filename, size_t index0, size_t index1);
    Array<float> x;
    Array<float> ki;
    Array<float> ke;
    Array<float> y;
};

}}
