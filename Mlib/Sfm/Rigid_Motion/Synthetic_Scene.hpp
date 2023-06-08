#pragma once
#include <Mlib/Array/Array.hpp>
#include <Mlib/Math/Transformation/Transformation_Matrix.hpp>

namespace Mlib::Sfm {

class SyntheticScene {
public:
    SyntheticScene(
        bool zero_first_extrinsic = false,
        float tR_multiplier = 1);
    TransformationMatrix<float, float, 3> delta_ke(size_t index0, size_t index1);
    FixedArray<float, 3> dt2(size_t index0, size_t index1);
    FixedArray<float, 3> dt(size_t index0, size_t index1);
    FixedArray<float, 3, 3> dR(size_t index0, size_t index1);
    void draw_to_bmp(const std::string& filename, size_t index0, size_t index1);
    Array<FixedArray<float, 3>> x;
    TransformationMatrix<float, float, 2> ki;
    Array<TransformationMatrix<float, float, 3>> ke;
    Array<FixedArray<float, 2>> y;
};

}
