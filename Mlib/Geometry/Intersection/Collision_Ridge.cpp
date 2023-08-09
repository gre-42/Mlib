#include "Collision_Ridge.hpp"
#include <Mlib/Geometry/Fixed_Cross.hpp>

using namespace Mlib;

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
    if (dot0d(tangent, other.normal) < 0.) {
        min_cos = RIDGE_UNTOUCHEABLE;
        return;
    }
    auto average_normal = (other.normal + normal);
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
    min_cos = dot0d(normal, other.normal);
    if (min_cos > max_min_cos_ridge) {
        min_cos = RIDGE_UNTOUCHEABLE;
    }
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
