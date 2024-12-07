#include "Collision_Ridge.hpp"
#include <Mlib/Geometry/Fixed_Cross.hpp>
#include <Mlib/Geometry/Physics_Material.hpp>

using namespace Mlib;

template <class TPosition>
FixedArray<SceneDir, 3> CollisionRidgeSphere<TPosition>::tangent() const {
    return cross(ray.direction, normal);
}

template <class TPosition>
bool CollisionRidgeSphere<TPosition>::is_touchable(SingleFaceBehavior behavior) const {
    if ((behavior == SingleFaceBehavior::UNTOUCHABLE) &&
        (min_cos == RIDGE_SINGLE_FACE) &&
        !any(physics_material & PhysicsMaterial::ATTR_TWO_SIDED))
    {
        return false;
    }
    return min_cos != RIDGE_UNTOUCHABLE;
}

template <class TPosition>
bool CollisionRidgeSphere<TPosition>::is_oriented() const {
    if (min_cos == RIDGE_SINGLE_FACE) {
        THROW_OR_ABORT("CollisionRidgeSphere has not been finalized");
    }
    if (min_cos == RIDGE_UNTOUCHABLE) {
        THROW_OR_ABORT("Collision attempt with untouchable CollisionRidgeSphere");
    }
    return min_cos < RIDGE_SPECIAL_THRESHOLD;
}

template <class TPosition>
void CollisionRidgeSphere<TPosition>::combine(
    const CollisionRidgeSphere<TPosition>& other,
    SceneDir max_min_cos_ridge)
{
    if (other.min_cos != RIDGE_SINGLE_FACE) {
        THROW_OR_ABORT("Ridge to be inserted has invalid state");
    }
    if (min_cos == RIDGE_360) {
        return;
    }
    if ((min_cos < RIDGE_SPECIAL_THRESHOLD) ||
        (min_cos == RIDGE_UNTOUCHABLE))
    {
        min_cos = RIDGE_360;
        lwarn() << "Creating 360 ridge";
        return;
    }
    if (min_cos != RIDGE_SINGLE_FACE) {
        THROW_OR_ABORT("Unknown ridge status");
    }
    auto other_normal_f = other.normal;
    bool ts0 = any(physics_material & PhysicsMaterial::ATTR_TWO_SIDED);
    bool ts1 = any(other.physics_material & PhysicsMaterial::ATTR_TWO_SIDED);
    if (ts0 != ts1) {
        physics_material |= PhysicsMaterial::ATTR_TWO_SIDED;
        ts0 = true;
        lwarn() << "Conflicting two-sidedness in collision ridges";
    }
    auto tang = tangent();
    if (auto c = dot0d(tang, other_normal_f); ts0) {
        if (std::abs(c) < max_min_cos_ridge) {
            min_cos = RIDGE_UNTOUCHABLE;
            return;
        }
        normal = -normal;
        if (dot0d(normal, other_normal_f) < 0.) {
            other_normal_f = -other_normal_f;
        }
    } else if (c < max_min_cos_ridge) {
        min_cos = RIDGE_UNTOUCHABLE;
        return;
    }
    auto average_normal = (other_normal_f + normal);
    auto len2 = sum(squared(average_normal));
    if (len2 < 1e-7) {
        normal = tang;
        min_cos = 0.;
        return;
    }
    normal = average_normal / std::sqrt(len2);
    min_cos = dot0d(normal, other_normal_f);
}

template <class TPosition>
void CollisionRidgeSphere<TPosition>::finalize() {
    if (min_cos == RIDGE_UNTOUCHABLE) {
        THROW_OR_ABORT("Attempt to finalize an untouchable ridge");
    }
    if (min_cos == RIDGE_SINGLE_FACE) {
        normal = tangent();
        min_cos = 0.;
    }
}

template <class TPosition>
CollisionRidgeSphere<TPosition> CollisionRidgeSphere<TPosition>::transformed(const TransformationMatrix<SceneDir, ScenePos, 3>& trafo) const {
    return CollisionRidgeSphere{
        bounding_sphere.template casted<ScenePos>().transformed(trafo).template casted<TPosition>(),
        physics_material,
        trafo.transform(edge.template casted<ScenePos>()).template casted<TPosition>(),
        ray.template casted<SceneDir, ScenePos>().transformed(trafo).template casted<SceneDir, TPosition>(),
        trafo.rotate(normal),
        min_cos
    };
}

namespace Mlib {

template struct CollisionRidgeSphere<HalfCompressedScenePos>;
template struct CollisionRidgeSphere<CompressedScenePos>;

}
