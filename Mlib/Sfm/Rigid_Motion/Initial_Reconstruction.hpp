#pragma once
#include <Mlib/Sfm/Rigid_Motion/Essential_Matrix_To_TR.h>
#include <Mlib/Sfm/Rigid_Motion/Normalized_Projection.hpp>

namespace Mlib { namespace Sfm {

class InitialReconstruction {
public:
    InitialReconstruction(
        const Array<float>& y0,
        const Array<float>& y1,
        const Array<float>& R,
        const Array<float>& t,
        const Array<float>& ki);

    Array<float> reconstructed(Array<float>* condition_number = nullptr) const;
    Array<float> projection_residual0() const;
    Array<float> projection_residual1() const;

private:
    Array<float> y0;
    Array<float> y1;
    Array<float> R;
    Array<float> t;
    Array<float> ki;
};

}}
