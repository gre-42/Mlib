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
        CollisionPolygonSphere<TResult, 3> dflt{
            .bounding_sphere = uninitialized,
            .polygon = uninitialized,
            .corners = uninitialized
        };
        RigidBodyAndCollisionTriangleSphere<TResult> result{ rb, dflt };
        std::visit([&result](auto& ctp){
            result.ctp = ctp.template casted<TResult>();
        }, ctp);
        return result;
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
    auto result = d;
    std::visit([&ref](auto& ctp){
        ctp = ctp + ref;
    }, result.ctp);
    return result;
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
    return { (a.aabb() - d).casted<HalfCompressedScenePos>(), (a.payload() - d).casted<HalfCompressedScenePos>() };
}

inline AabbAndPayload<CompressedScenePos, 3, RigidBodyAndCollisionTriangleSphere<CompressedScenePos>>
    decompress(
        const AabbAndPayload<HalfCompressedScenePos, 3, RigidBodyAndCollisionTriangleSphere<HalfCompressedScenePos>>& a,
        const FixedArray<CompressedScenePos, 3>& d)
{
    return {
        a.aabb().casted<CompressedScenePos>() + d,
        a.payload().casted<CompressedScenePos>() + d};
}

}
