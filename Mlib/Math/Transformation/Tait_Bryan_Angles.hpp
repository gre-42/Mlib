#pragma once
#include <Mlib/Math/Fixed_Rodrigues.hpp>
#include <Mlib/Math/Transformation/Transformation_Matrix.hpp>

namespace Mlib {

template <class TRotation, class TPos, size_t tsize>
class OffsetAndTaitBryanAngles {
public:
    static OffsetAndTaitBryanAngles identity() {
        return OffsetAndTaitBryanAngles{
            fixed_zeros<TRotation>(),
            fixed_zeros<TPos>()};
    };
    OffsetAndTaitBryanAngles(Uninitialized)
        : rotation{ uninitialized }
        , position{ uninitialized }
    {}
    OffsetAndTaitBryanAngles(
        const FixedArray<TRotation, tsize>& rotation,
        const FixedArray<TPos, tsize>& position)
        : rotation{ rotation }
        , position{ position }
    {}
    OffsetAndTaitBryanAngles(
        const FixedArray<TRotation, tsize, tsize>& rotation,
        const FixedArray<TPos, tsize>& position)
        : rotation{ matrix_2_tait_bryan_angles(rotation) }
        , position{ position }
    {}

    TransformationMatrix<TRotation, TPos, tsize> to_matrix() const {
        return TransformationMatrix<TRotation, TPos, tsize>{
            tait_bryan_angles_2_matrix(rotation),
            position};
    }

    FixedArray<TRotation, tsize> rotation;
    FixedArray<TPos, tsize> position;
};

};
