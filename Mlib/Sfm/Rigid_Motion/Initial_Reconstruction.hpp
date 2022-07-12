#pragma once
#include <Mlib/Sfm/Rigid_Motion/Essential_Matrix_To_TR.h>
#include <Mlib/Sfm/Rigid_Motion/Normalized_Projection.hpp>

namespace Mlib::Sfm {

class InitialReconstruction {
public:
    InitialReconstruction(
        const Array<FixedArray<float, 2>>& y0,
        const Array<FixedArray<float, 2>>& y1,
        const TransformationMatrix<float, float, 3>& ke,
        const TransformationMatrix<float, float, 2>& ki);

    Array<FixedArray<float, 3>> reconstructed(Array<float>* condition_number = nullptr) const;
    Array<FixedArray<float, 2>> projection_residual0() const;
    Array<FixedArray<float, 2>> projection_residual1() const;

private:
    Array<FixedArray<float, 2>> y0;
    Array<FixedArray<float, 2>> y1;
    TransformationMatrix<float, float, 3> ke;
    TransformationMatrix<float, float, 2> ki;
};

}
