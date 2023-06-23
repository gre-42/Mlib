#include "Collision_Ridges_Base.hpp"
#include <Mlib/Math/Orderable_Fixed_Array.hpp>

using namespace Mlib;

std::pair<OrderableFixedArray<double, 3>, OrderableFixedArray<double, 3>> OrderableRidgeSphereBase::key() const
{
    if (OrderableFixedArray{collision_ridge_sphere.edge(0)} > OrderableFixedArray{collision_ridge_sphere.edge(1)}) {
        return std::make_pair(
            OrderableFixedArray{collision_ridge_sphere.edge(0)},
            OrderableFixedArray{collision_ridge_sphere.edge(1)});
    } else {
        return std::make_pair(
            OrderableFixedArray{collision_ridge_sphere.edge(1)},
            OrderableFixedArray{collision_ridge_sphere.edge(0)});
    }
}

bool OrderableRidgeSphereBase::operator < (const OrderableRidgeSphereBase& other) const {
    return key() < other.key();
}
