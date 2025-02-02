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
        : rotation_{ uninitialized }
        , position_{ uninitialized }
    {}
    OffsetAndTaitBryanAngles(
        const FixedArray<TRotation, tsize>& rotation,
        const FixedArray<TPos, tsize>& position)
        : rotation_{ rotation }
        , position_{ position }
    {}
    OffsetAndTaitBryanAngles(
        const FixedArray<TRotation, tsize, tsize>& rotation,
        const FixedArray<TPos, tsize>& position)
        : rotation_{ matrix_2_tait_bryan_angles(rotation) }
        , position_{ position }
    {}

    TransformationMatrix<TRotation, TPos, tsize> to_matrix() const {
        return TransformationMatrix<TRotation, TPos, tsize>{
            tait_bryan_angles_2_matrix(rotation_),
            position_};
    }

    FixedArray<TRotation, tsize>& rotation() { return rotation_; }
    const FixedArray<TRotation, tsize>& rotation() const { return rotation_; }
    FixedArray<TPos, tsize>& position() { return position_; }
    const FixedArray<TPos, tsize>& position() const { return position_; }

    TRotation& rotation(size_t i) { return rotation_(i); }
    const TRotation& rotation(size_t i) const { return rotation_(i); }
    TPos& position(size_t i) { return position_(i); }
    const TPos& position(size_t i) const { return position_(i); }
private:
    FixedArray<TRotation, tsize> rotation_;
    FixedArray<TPos, tsize> position_;
};

};
