#pragma once
#include <Mlib/Geometry/Mesh/Collision_Ridges_Base.hpp>

namespace Mlib {

template <class TData, size_t... tshape>
class OrderableFixedArray;
enum class PhysicsMaterial;

class CollisionRidges: public CollisionRidgesBase<OrderableRidgeSphereBase> {
public:
    CollisionRidges();
    ~CollisionRidges();
    void insert(
        const FixedArray<FixedArray<double, 3>, 3>& tri,
        const FixedArray<double, 3>& normal,
        double max_min_cos_ridge,
        PhysicsMaterial physics_material);
protected:
    void insert(
        const FixedArray<double, 3>& a,
        const FixedArray<double, 3>& b,
        const FixedArray<double, 3>& normal,
        double max_min_cos_ridge,
        PhysicsMaterial physics_material);
};

}
