#pragma once
#include <Mlib/Geometry/Intersection/Collision_Ridge.hpp>
#include <cstddef>
#include <set>

namespace Mlib {

template <class TData, size_t... tshape>
class OrderableFixedArray;
enum class PhysicsMaterial;

struct OrderableRidgeSphere {
    CollisionRidgeSphere collision_ridge_sphere;
    std::pair<OrderableFixedArray<double, 3>, OrderableFixedArray<double, 3>> key() const;
    bool operator < (const OrderableRidgeSphere& other) const;
};

class CollisionRidges {
public:
    CollisionRidges();
    ~CollisionRidges();
    using Edges = std::set<OrderableRidgeSphere>;
    using const_iterator = Edges::const_iterator;
    void insert(
        const FixedArray<FixedArray<double, 3>, 3>& tri,
        const FixedArray<double, 3>& normal,
        double max_min_cos_ridge,
        PhysicsMaterial physics_material);
    const_iterator begin() const;
    const_iterator end() const;
    size_t size() const;
private:
    void insert(
        const FixedArray<double, 3>& a,
        const FixedArray<double, 3>& b,
        const FixedArray<double, 3>& normal,
        double max_min_cos_ridge,
        PhysicsMaterial physics_material);
    Edges edges_;
};

}
