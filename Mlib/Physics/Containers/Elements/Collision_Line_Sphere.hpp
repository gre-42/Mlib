#pragma once
#include <Mlib/Geometry/Intersection/Bvh.hpp>
#include <Mlib/Geometry/Intersection/Collision_Line.hpp>
#include <Mlib/Scene_Config/Scene_Precision.hpp>

namespace Mlib {

class RigidBodyVehicle;

template <class TPosition>
struct RigidBodyAndCollisionLineSphere {
    RigidBodyVehicle& rb;
    CollisionLineSphere<TPosition> clp;
    template <class TPosition2>
    RigidBodyAndCollisionLineSphere<TPosition2> casted() const {
        return { rb, clp.template casted<TPosition2>() };
    }
    bool operator == (const RigidBodyAndCollisionLineSphere& other) const {
        return (&rb == &other.rb) &&
               (clp == other.clp);
    }
};

inline RigidBodyAndCollisionLineSphere<CompressedScenePos>
    operator + (
        const RigidBodyAndCollisionLineSphere<CompressedScenePos>& d,
        const FixedArray<CompressedScenePos, 3>& ref)
{
    return { d.rb, d.clp + ref };
}

inline RigidBodyAndCollisionLineSphere<CompressedScenePos>
    operator - (
        const RigidBodyAndCollisionLineSphere<CompressedScenePos>& d,
        const FixedArray<CompressedScenePos, 3>& ref)
{
    return d + (-ref);
}

inline AabbAndPayload<HalfCompressedScenePos, 3, RigidBodyAndCollisionLineSphere<HalfCompressedScenePos>>
    compress(
        const AabbAndPayload<CompressedScenePos, 3, RigidBodyAndCollisionLineSphere<CompressedScenePos>>& a,
        const FixedArray<CompressedScenePos, 3>& d)
{
    return { (a.primitive() - d).casted<HalfCompressedScenePos>(), (a.payload() - d).casted<HalfCompressedScenePos>() };
}

inline AabbAndPayload<CompressedScenePos, 3, RigidBodyAndCollisionLineSphere<CompressedScenePos>>
    decompress(
        const AabbAndPayload<HalfCompressedScenePos, 3, RigidBodyAndCollisionLineSphere<HalfCompressedScenePos>>& a,
        const FixedArray<CompressedScenePos, 3>& d)
{
    return {
        a.primitive().casted<CompressedScenePos>() + d,
        a.payload().casted<CompressedScenePos>() + d};
}

}
