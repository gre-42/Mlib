#pragma once
#include <Mlib/Geometry/Intersection/Collision_Ridge.hpp>
#include <cstddef>
#include <set>

namespace Mlib {

template <class TData, size_t tshape0, size_t... tshape>
class OrderableFixedArray;

struct OrderableRidgeSphereBase {
    CollisionRidgeSphere collision_ridge_sphere;
    std::pair<OrderableFixedArray<ScenePos, 3>, OrderableFixedArray<ScenePos, 3>> key() const;
    bool operator < (const OrderableRidgeSphereBase& other) const;
};

template <class TOrderableRidgeSphere>
class CollisionRidgesBase {
public:
    CollisionRidgesBase();
    ~CollisionRidgesBase();
    using Ridges = std::set<TOrderableRidgeSphere>;
    using const_iterator = typename Ridges::const_iterator;
    using node_type = typename Ridges::node_type;
    const_iterator begin() const;
    const_iterator end() const;
    node_type extract(const_iterator it);
    size_t size() const;
    bool empty() const;
    void clear();
protected:
    void insert(
        const TOrderableRidgeSphere& ridge,
        ScenePos max_min_cos_ridge);
private:
    Ridges ridges_;
};

}
