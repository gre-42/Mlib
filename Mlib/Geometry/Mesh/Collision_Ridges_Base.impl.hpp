#include "Collision_Ridges_Base.hpp"
#include <Mlib/Geometry/Exceptions/Edge_Exception.hpp>
#include <Mlib/Geometry/Fixed_Cross.hpp>

using namespace Mlib;

template <class TOrderableRidgeSphere>
CollisionRidgesBase<TOrderableRidgeSphere>::CollisionRidgesBase() = default;

template <class TOrderableRidgeSphere>
CollisionRidgesBase<TOrderableRidgeSphere>::~CollisionRidgesBase() = default;

template <class TOrderableRidgeSphere>
void CollisionRidgesBase<TOrderableRidgeSphere>::insert(
    const TOrderableRidgeSphere& ridge,
    double max_min_cos_ridge)
{
    auto previous_ridge = ridges_.find(ridge);
    if (previous_ridge == ridges_.end()) {
        if (!ridges_.insert(ridge).second) {
            THROW_OR_ABORT("CollisionRidges::insert internal error");
        }
    } else {
        const auto& ridge_normal = ridge.collision_ridge_sphere.normal;
        auto& old_ridge = const_cast<CollisionRidgeSphere&>(previous_ridge->collision_ridge_sphere);
        if (!std::isnan(old_ridge.min_cos)) {
            EdgeException<double> exc{
                ridge.collision_ridge_sphere.edge(0),
                ridge.collision_ridge_sphere.edge(1),
                "Detected duplicate triangles touching the same ridge"};
            THROW_OR_ABORT2(exc);
        }
        if (dot0d(cross(old_ridge.edge(1) - old_ridge.edge(0), old_ridge.normal), ridge_normal) < 0) {
            ridges_.erase(previous_ridge);
            return;
        }
        auto average_normal = (ridge_normal + old_ridge.normal);
        auto len2 = sum(squared(average_normal));
        if (len2 < 1e-7) {
            EdgeException<double> exc{
                ridge.collision_ridge_sphere.edge(0),
                ridge.collision_ridge_sphere.edge(1),
                "Detected parallel triangles with opposing faces"};
            THROW_OR_ABORT2(exc);
        }
        old_ridge.normal = average_normal / std::sqrt(len2);
        old_ridge.min_cos = dot0d(old_ridge.normal, ridge_normal);
        if (old_ridge.min_cos > max_min_cos_ridge) {
            ridges_.erase(previous_ridge);
        }
    }
}

template <class TOrderableRidgeSphere>
typename CollisionRidgesBase<TOrderableRidgeSphere>::const_iterator CollisionRidgesBase<TOrderableRidgeSphere>::begin() const {
    return ridges_.begin();
}

template <class TOrderableRidgeSphere>
typename CollisionRidgesBase<TOrderableRidgeSphere>::const_iterator CollisionRidgesBase<TOrderableRidgeSphere>::end() const {
    return ridges_.end();
}

template <class TOrderableRidgeSphere>
size_t CollisionRidgesBase<TOrderableRidgeSphere>::size() const {
    return ridges_.size();
}

template <class TOrderableRidgeSphere>
void CollisionRidgesBase<TOrderableRidgeSphere>::clear() {
    return ridges_.clear();
}
