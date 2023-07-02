#pragma once
#include <Mlib/Geometry/Intersection/Collision_Ridge.hpp>
#include <cstddef>
#include <set>

namespace Mlib {

template <class TData, size_t... tshape>
class OrderableFixedArray;
enum class CollisionRidgeErrorBehavior;

struct OrderableRidgeSphereBase {
    CollisionRidgeSphere collision_ridge_sphere;
    std::pair<OrderableFixedArray<double, 3>, OrderableFixedArray<double, 3>> key() const;
    bool operator < (const OrderableRidgeSphereBase& other) const;
};

template <class TOrderableRidgeSphere>
class CollisionRidgesBase {
public:
    CollisionRidgesBase();
    ~CollisionRidgesBase();
    using Ridges = std::set<TOrderableRidgeSphere>;
    using const_iterator = typename Ridges::const_iterator;
    const_iterator begin() const;
    const_iterator end() const;
    size_t size() const;
    void clear();
protected:
    void insert(
        const TOrderableRidgeSphere& ridge,
        double max_min_cos_ridge,
        CollisionRidgeErrorBehavior error_behavior);
private:
    Ridges ridges_;
};

}
