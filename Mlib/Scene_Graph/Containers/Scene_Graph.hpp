#pragma once
#include <Mlib/Geometry/Mesh/Typed_Mesh.hpp>
#include <Mlib/Geometry/Primitives/Bvh.hpp>
#include <Mlib/Geometry/Primitives/Bvh_Grid.hpp>
#include <Mlib/Physics/Containers/Elements/Collision_Triangle_Sphere.hpp>
#include <Mlib/Scene_Config/Scene_Precision.hpp>
#include <Mlib/Scene_Graph/Elements/Scene_Node.hpp>
#include <Mlib/Scene_Graph/Scene_Element_Type.hpp>
#include <memory>
#include <set>

namespace Mlib {

class Camera;
class IRenderable;
struct DeferredRenderable;
struct RenderEngineConfig;
struct PhysicsEngineConfig;
class RigidBodyVehicle;
class IIntersectableMesh;

struct RigidBodyAndIntersectableMesh {
    std::unique_ptr<SceneNode> node;
    std::shared_ptr<RigidBodyVehicle> rb;
    TypedMesh<std::shared_ptr<IIntersectableMesh>> mesh;
};

class SharedIRenderable {
public:
    explicit SharedIRenderable(
        const SharedIRenderable& element)
        : element_{ element.element_ }
    {}
private:
    std::shared_ptr<IRenderable> element_;
};

class SharedRigidBodyAndIntersectableMesh {
public:
    explicit SharedRigidBodyAndIntersectableMesh(
        const SharedRigidBodyAndIntersectableMesh& element)
        : element_{ element.element_ }
    {}
private:
    std::shared_ptr<RigidBodyAndIntersectableMesh> element_;
};

class SceneGraph {
public:
    explicit SceneGraph(
        const RenderEngineConfig& rcfg,
        const PhysicsEngineConfig& pcfg);
    ~SceneGraph();
    void render(const Camera& camera);
    void move(SceneElementTypes types);
private:
    using RenderablesBvh = Bvh<
        CompressedScenePos,
        3,
        SharedIRenderable,
        BvhThreadSafety::THREAD_SAFE>;
    using CollidablePolygonBvh = CompressedBvhGrid<
        CompressedScenePos,
        HalfCompressedScenePos,
        RigidBodyAndCollisionTriangleSphere<CompressedScenePos>,
        RigidBodyAndCollisionTriangleSphere<HalfCompressedScenePos>,
        3>;
    using CollidableMovablesBvh = BvhGrid<
        CompressedScenePos,
        3,
        SharedRigidBodyAndIntersectableMesh>;
    void render(
        const FixedArray<float, 4, 4>& vp,
        const FixedArray<ScenePos, 3>& offset,
        std::set<DeferredRenderable>& deferred,
        SceneElementTypes types);
    RenderablesBvh dynamic_renderables_;
    RenderablesBvh static_renderables_;
    CollidableMovablesBvh dynamic_object_collidables_;
    CollidablePolygonBvh static_polygonal_collidables_;
};

}
