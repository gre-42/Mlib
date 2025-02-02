#include "Collision_Ridges_Base.hpp"
#include <Mlib/Geometry/Exceptions/Edge_Exception.hpp>
#include <Mlib/Type_Traits/Remove_Const.hpp>

namespace Mlib {

template <class TOrderableRidgeSphere>
CollisionRidgesBase<TOrderableRidgeSphere>::CollisionRidgesBase() = default;

template <class TOrderableRidgeSphere>
CollisionRidgesBase<TOrderableRidgeSphere>::~CollisionRidgesBase() = default;

template <class TOrderableRidgeSphere>
void CollisionRidgesBase<TOrderableRidgeSphere>::insert(
    const TOrderableRidgeSphere& ridge,
    SceneDir max_min_cos_ridge)
{
    const auto& ridge_normal = ridge.collision_ridge_sphere.normal;
    if (all(abs(ridge_normal) < 1e-12f)) {
        THROW_OR_ABORT("Ridge has triangle-normal close to or equal 0");
    }
    if (ridge.collision_ridge_sphere.min_cos != RIDGE_SINGLE_FACE) {
        THROW_OR_ABORT("Ridge min_cos is not RIDGE_SINGLE_FACE");
    }
    auto previous_ridge = ridges_.find(ridge);
    if (previous_ridge == ridges_.end()) {
        if (!ridges_.insert(ridge).second) {
            THROW_OR_ABORT("CollisionRidges::insert internal error or data race");
        }
        return;
    }
    remove_const(previous_ridge->collision_ridge_sphere).combine(
        ridge.collision_ridge_sphere,
        max_min_cos_ridge);
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
typename CollisionRidgesBase<TOrderableRidgeSphere>::node_type CollisionRidgesBase<TOrderableRidgeSphere>::extract(const_iterator it) {
    return ridges_.extract(it);
}

template <class TOrderableRidgeSphere>
size_t CollisionRidgesBase<TOrderableRidgeSphere>::size() const {
    return ridges_.size();
}

template <class TOrderableRidgeSphere>
bool CollisionRidgesBase<TOrderableRidgeSphere>::empty() const {
    return ridges_.empty();
}

template <class TOrderableRidgeSphere>
void CollisionRidgesBase<TOrderableRidgeSphere>::clear() {
    return ridges_.clear();
}

}
