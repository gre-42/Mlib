#include "Collision_Ridge.hpp"
#include <Mlib/Geometry/Fixed_Cross.hpp>
#include <Mlib/Geometry/Physics_Material.hpp>

using namespace Mlib;

bool CollisionRidgeSphere::is_touchable(SingleFaceBehavior behavior) const {
    if ((behavior == SingleFaceBehavior::UNTOUCHEABLE) &&
        (min_cos == RIDGE_SINGLE_FACE))
    {
        return false;
    }
    return min_cos != RIDGE_UNTOUCHEABLE;
}

bool CollisionRidgeSphere::is_oriented() const {
    if (min_cos == RIDGE_SINGLE_FACE) {
        THROW_OR_ABORT("CollisionRidgeSphere has not been finalized");
    }
    if (min_cos == RIDGE_UNTOUCHEABLE) {
        THROW_OR_ABORT("Collision attempt with untouchable CollisionRidgeSphere");
    }
    return min_cos < RIDGE_SPECIAL_THRESHOLD;
}

void CollisionRidgeSphere::combine(
    const CollisionRidgeSphere& other,
    double max_min_cos_ridge)
{
    if (other.min_cos != RIDGE_SINGLE_FACE) {
        THROW_OR_ABORT("Ridge to be inserted has invalid state");
    }
    if (min_cos == RIDGE_360) {
        return;
    }
    if ((min_cos == RIDGE_SINGLE_FACE) ||
        (min_cos == RIDGE_UNTOUCHEABLE))
    {
        min_cos = RIDGE_360;
        return;
    }
    if (min_cos != RIDGE_SINGLE_FACE) {
        THROW_OR_ABORT("Unknown ridge status");
    }
    auto tangent = cross(edge(1) - edge(0), normal);
    auto other_normal_f = other.normal;
    if (dot0d(tangent, other_normal_f) < max_min_cos_ridge) {
        bool ts0 = any(physics_material & PhysicsMaterial::ATTR_TWO_SIDED);
        bool ts1 = any(other.physics_material & PhysicsMaterial::ATTR_TWO_SIDED);
        if (ts0 != ts1) {
            THROW_OR_ABORT("Conflicting two-sidedness in collision ridges");
        }
        if (!ts0) {
            min_cos = RIDGE_UNTOUCHEABLE;
            return;
        } else {
            normal = -normal;
            other_normal_f = -other_normal_f;
        }
    }
    auto average_normal = (other_normal_f + normal);
    auto len2 = sum(squared(average_normal));
    if (len2 < 1e-7) {
        auto tlen2 = sum(squared(tangent));
        if (tlen2 < 1e-12) {
            THROW_OR_ABORT("Ridge tangent is close to zero");
        }
        normal = tangent / std::sqrt(tlen2);
        min_cos = 0.;
        return;
    }
    normal = average_normal / std::sqrt(len2);
    min_cos = dot0d(normal, other_normal_f);
}

void CollisionRidgeSphere::finalize() {
    if (min_cos == RIDGE_UNTOUCHEABLE) {
        THROW_OR_ABORT("Attempt to finalize an untouchable ridge");
    }
    if (min_cos == RIDGE_SINGLE_FACE) {
        auto tangent = cross(edge(1) - edge(0), normal);
        auto tlen2 = sum(squared(tangent));
        if (tlen2 < 1e-12) {
            THROW_OR_ABORT("Ridge tangent is close to zero");
        }
        normal = tangent / std::sqrt(tlen2);
        min_cos = 0.;
    }
}
