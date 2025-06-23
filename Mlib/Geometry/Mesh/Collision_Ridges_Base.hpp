#pragma once
#include <Mlib/Geometry/Intersection/Collision_Ridge.hpp>
#include <Mlib/Hash.hpp>
#include <Mlib/Math/Orderable_Fixed_Array_Hash.hpp>
#include <compare>
#include <cstddef>
#include <unordered_set>

namespace Mlib {

template <class TData, size_t... tshape>
class OrderableFixedArray;

template <class TPosition>
struct OrderableRidgeSphereBase {
public:
    OrderableRidgeSphereBase(const CollisionRidgeSphere<TPosition>& value)
        : collision_ridge_sphere{ value }
        , hash{ key() }
    {}
    CollisionRidgeSphere<TPosition> collision_ridge_sphere;
    size_t hash;
    inline bool operator == (const OrderableRidgeSphereBase& other) const {
        return hash == other.hash;
    }
private:
    inline size_t key() const {
        if (make_orderable(collision_ridge_sphere.edge[0]) > make_orderable(collision_ridge_sphere.edge[1])) {
            return hash_combine(
                make_orderable(collision_ridge_sphere.edge[0]),
                make_orderable(collision_ridge_sphere.edge[1]));
        } else {
            return hash_combine(
                make_orderable(collision_ridge_sphere.edge[1]),
                make_orderable(collision_ridge_sphere.edge[0]));
        }
    }
};

template <class TOrderableRidgeSphere>
class CollisionRidgesBase {
public:
    CollisionRidgesBase();
    ~CollisionRidgesBase();
    using Ridges = std::unordered_set<TOrderableRidgeSphere>;
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
        SceneDir max_min_cos_ridge);
private:
    Ridges ridges_;
};

}

namespace std {

template<class TPosition>
struct hash<Mlib::OrderableRidgeSphereBase<TPosition>>
{
    std::size_t operator()(const Mlib::OrderableRidgeSphereBase<TPosition>& s) const noexcept {
        return s.hash;
    }
};

}
