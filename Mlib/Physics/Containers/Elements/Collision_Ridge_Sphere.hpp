#pragma once
#include <Mlib/Geometry/Intersection/Bvh.hpp>
#include <Mlib/Geometry/Intersection/Collision_Ridge.hpp>
#include <Mlib/Scene_Config/Scene_Precision.hpp>

namespace Mlib {

class RigidBodyVehicle;

template <class TPosition>
struct RigidBodyAndCollisionRidgeSphere {
    RigidBodyVehicle& rb;
    CollisionRidgeSphere<TPosition> crp;
    template <class TPosition2>
    RigidBodyAndCollisionRidgeSphere<TPosition2> casted() const {
        return { rb, crp.template casted<TPosition2>() };
    }
    bool operator == (const RigidBodyAndCollisionRidgeSphere& other) const {
        return (&rb == &other.rb) &&
               (crp == other.crp);
    }
};

inline RigidBodyAndCollisionRidgeSphere<CompressedScenePos>
    operator + (
        const RigidBodyAndCollisionRidgeSphere<CompressedScenePos>& d,
        const FixedArray<CompressedScenePos, 3>& ref)
{
    return { d.rb, d.crp + ref };
}

inline RigidBodyAndCollisionRidgeSphere<CompressedScenePos>
    operator - (
        const RigidBodyAndCollisionRidgeSphere<CompressedScenePos>& d,
        const FixedArray<CompressedScenePos, 3>& ref)
{
    return d + (-ref);
}

inline AabbAndPayload<HalfCompressedScenePos, 3, RigidBodyAndCollisionRidgeSphere<HalfCompressedScenePos>>
    compress(
        const AabbAndPayload<CompressedScenePos, 3, RigidBodyAndCollisionRidgeSphere<CompressedScenePos>>& a,
        const FixedArray<CompressedScenePos, 3>& d)
{
    return { (a.primitive() - d).casted<HalfCompressedScenePos>(), (a.payload() - d).casted<HalfCompressedScenePos>() };
}

inline AabbAndPayload<CompressedScenePos, 3, RigidBodyAndCollisionRidgeSphere<CompressedScenePos>>
    decompress(
        const AabbAndPayload<HalfCompressedScenePos, 3, RigidBodyAndCollisionRidgeSphere<HalfCompressedScenePos>>& a,
        const FixedArray<CompressedScenePos, 3>& d)
{
    return {
        a.primitive().casted<CompressedScenePos>() + d,
        a.payload().casted<CompressedScenePos>() + d};
}

}
