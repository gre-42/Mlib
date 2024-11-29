#pragma once
#include <Mlib/Scene_Precision.hpp>
#include <cstddef>

namespace Mlib {

template <typename TData, size_t... tshape>
class FixedArray;

class ICollisionNormalModifier {
public:
    virtual ~ICollisionNormalModifier() = default;
    virtual void modify_collision_normal(
        const FixedArray<ScenePos, 3>& position,
        FixedArray<float, 3>& normal,
        float& overlap) const = 0;
};

}
