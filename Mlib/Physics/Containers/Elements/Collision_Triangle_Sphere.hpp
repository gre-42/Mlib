#pragma once
#include <Mlib/Geometry/Intersection/Bvh.hpp>
#include <Mlib/Geometry/Intersection/Collision_Polygon.hpp>
#include <Mlib/Scene_Precision.hpp>

namespace Mlib {

class RigidBodyVehicle;

template <class TPosition>
struct RigidBodyAndCollisionTriangleSphere {
    RigidBodyVehicle& rb;
    std::variant<CollisionPolygonSphere<TPosition, 3>, CollisionPolygonSphere<TPosition, 4>> ctp;
    template <class TResult>
    RigidBodyAndCollisionTriangleSphere<TResult> casted() const {
        std::optional<RigidBodyAndCollisionTriangleSphere<TResult>> result;
        std::visit([&](auto& ctp){
            result.emplace(rb, ctp.template casted<TResult>());
        }, ctp);
        return *result;
    }
    bool operator == (const RigidBodyAndCollisionTriangleSphere& other) const {
        return (&rb == &other.rb) &&
               (ctp == other.ctp);
    }
};

inline RigidBodyAndCollisionTriangleSphere<CompressedScenePos>
    operator + (
        const RigidBodyAndCollisionTriangleSphere<CompressedScenePos>& d,
        const FixedArray<CompressedScenePos, 3>& ref)
{
    std::optional<RigidBodyAndCollisionTriangleSphere<CompressedScenePos>> result;
    std::visit([&](auto& ctp){
        result.emplace(d.rb, ctp + ref);
    }, d.ctp);
    return *result;
}

inline RigidBodyAndCollisionTriangleSphere<CompressedScenePos>
    operator - (
        const RigidBodyAndCollisionTriangleSphere<CompressedScenePos>& d,
        const FixedArray<CompressedScenePos, 3>& ref)
{
    return d + (-ref);
}

inline AabbAndPayload<HalfCompressedScenePos, 3, RigidBodyAndCollisionTriangleSphere<HalfCompressedScenePos>>
    compress(
        const AabbAndPayload<CompressedScenePos, 3, RigidBodyAndCollisionTriangleSphere<CompressedScenePos>>& a,
        const FixedArray<CompressedScenePos, 3>& d)
{
    return { (a.primitive() - d).casted<HalfCompressedScenePos>(), (a.payload() - d).casted<HalfCompressedScenePos>() };
}

inline AabbAndPayload<CompressedScenePos, 3, RigidBodyAndCollisionTriangleSphere<CompressedScenePos>>
    decompress(
        const AabbAndPayload<HalfCompressedScenePos, 3, RigidBodyAndCollisionTriangleSphere<HalfCompressedScenePos>>& a,
        const FixedArray<CompressedScenePos, 3>& d)
{
    return {
        a.primitive().casted<CompressedScenePos>() + d,
        a.payload().casted<CompressedScenePos>() + d};
}

}
