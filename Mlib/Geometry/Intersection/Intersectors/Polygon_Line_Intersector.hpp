#include <Mlib/Scene_Pos.hpp>
#include <memory>

namespace Mlib {

class IIntersectionInfo;
template <class TData, size_t tnvertices>
struct CollisionPolygonSphere;
template <class TData>
struct CollisionRidgeSphere;
template <class TData>
class IIntersectable;
template <class TData>
struct CollisionLineSphere;
template <class TDir, class TPos, size_t n>
class TransformationMatrix;

// Quad - ridge
std::unique_ptr<IIntersectionInfo> intersect(
    const CollisionPolygonSphere<ScenePos, 4>& q0,
    const CollisionRidgeSphere<ScenePos>& r1);

// Triangle - ridge
std::unique_ptr<IIntersectionInfo> intersect(
    const CollisionPolygonSphere<ScenePos, 3>& t0,
    const CollisionRidgeSphere<ScenePos>& r1);

// Quad - line
std::unique_ptr<IIntersectionInfo> intersect(
    const CollisionPolygonSphere<ScenePos, 4>& q0,
    const CollisionLineSphere<ScenePos>& l1);

// Triangle - line
std::unique_ptr<IIntersectionInfo> intersect(
    const CollisionPolygonSphere<ScenePos, 3>& t0,
    const CollisionLineSphere<ScenePos>& l1);

// Quad - intersectable
std::unique_ptr<IIntersectionInfo> intersect(
    const CollisionPolygonSphere<ScenePos, 4>& q0,
    const IIntersectable<ScenePos>& i1);

// Triangle - intersectable
std::unique_ptr<IIntersectionInfo> intersect(
    const CollisionPolygonSphere<ScenePos, 3>& t0,
    const IIntersectable<ScenePos>& i1);

// Intersectable - ridge
std::unique_ptr<IIntersectionInfo> intersect(
    const IIntersectable<ScenePos>& i0,
    const CollisionRidgeSphere<ScenePos>& r1);

// Intersectable - line
std::unique_ptr<IIntersectionInfo> intersect(
    const IIntersectable<ScenePos>& i0,
    const CollisionLineSphere<ScenePos>& l1);

// Intersectable - intersectable
std::unique_ptr<IIntersectionInfo> intersect(
    const IIntersectable<ScenePos>& i0,
    const IIntersectable<ScenePos>& i1);

}
