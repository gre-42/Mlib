#include "Rigid_Bodies.hpp"
#include <Mlib/Assert.hpp>
#include <Mlib/Geometry/Colored_Vertex.hpp>
#include <Mlib/Geometry/Intersection/Welzl.hpp>
#include <Mlib/Geometry/Mesh/Colored_Vertex_Array.hpp>
#include <Mlib/Geometry/Mesh/Lazy_Transformed_Mesh.hpp>
#include <Mlib/Geometry/Mesh/Static_Transformed_Mesh.hpp>
#include <Mlib/Geometry/Physics_Material.hpp>
#include <Mlib/Images/Svg.hpp>
#include <Mlib/Physics/Collision/Collidable_Mode.hpp>
#include <Mlib/Physics/Physics_Engine/Physics_Engine_Config.hpp>
#include <Mlib/Physics/Rigid_Body/Rigid_Body_Vehicle.hpp>
#include <Mlib/Scene_Graph/Physics_Resource_Filter.hpp>
#include <Mlib/Throw_Or_Abort.hpp>

using namespace Mlib;

RigidBodies::RigidBodies(const PhysicsEngineConfig& cfg)
: convex_mesh_bvh_{{cfg.bvh_max_size, cfg.bvh_max_size, cfg.bvh_max_size}, cfg.bvh_levels},
  triangle_bvh_{{cfg.bvh_max_size, cfg.bvh_max_size, cfg.bvh_max_size}, cfg.bvh_levels},
  line_bvh_{{cfg.bvh_max_size, cfg.bvh_max_size, cfg.bvh_max_size}, cfg.bvh_levels}
{}

void RigidBodies::add_rigid_body(
    const std::shared_ptr<RigidBodyVehicle>& rigid_body,
    const std::list<std::shared_ptr<ColoredVertexArray<float>>>& s_hitboxes,
    const std::list<std::shared_ptr<ColoredVertexArray<double>>>& d_hitboxes,
    CollidableMode collidable_mode,
    const PhysicsResourceFilter& physics_resource_filter)
{
    auto rng = welzl_rng();
    if (collidable_mode == CollidableMode::TERRAIN) {
        if (rigid_body->mass() != INFINITY) {
            THROW_OR_ABORT("Terrain requires infinite mass");
        }
        // if (!tirelines.empty()) {
        //     THROW_OR_ABORT("static rigid body has tirelines");
        // }
        auto add_hitboxes = [&]<typename TPos>(const std::list<std::shared_ptr<ColoredVertexArray<TPos>>>& hitboxes) {
            for (auto& m : hitboxes) {
                if (physics_resource_filter.matches(*m)) {
                    if (any(m->physics_material & PhysicsMaterial::OBJ_GRIND_LINE)) {
                        for (const auto& t : m->transformed_lines_bbox(rigid_body->get_new_absolute_model_matrix())) {
                            line_bvh_.insert(t.aabb, {*rigid_body, t.base});
                        }
                    } else {
                        if (any(m->physics_material & PhysicsMaterial::ATTR_CONVEX)) {
                            auto transformed = m->transformed_triangles_bbox(rigid_body->get_new_absolute_model_matrix());
                            std::set<OrderableFixedArray<double, 3>> vertex_set;
                            std::vector<const FixedArray<double, 3>*> vertex_vector;
                            vertex_vector.reserve(3 * transformed.size());
                            for (const CollisionTriangleAabb& t : transformed) {
                                for (const auto& v : t.base.triangle.flat_iterable()) {
                                    if (vertex_set.insert(OrderableFixedArray{v}).second) {
                                        vertex_vector.push_back(&v);
                                    }
                                }
                            }
                            AxisAlignedBoundingBox<double, 3> aabb(vertex_set.begin(), vertex_set.end());
                            BoundingSphere<double, 3> bounding_sphere = welzl_from_vector<double, 3>(vertex_vector, rng);
                            std::vector<CollisionTriangleSphere> triangles;
                            std::vector<CollisionLineSphere> lines;
                            triangles.reserve(transformed.size());
                            for (const CollisionTriangleAabb& t : transformed) {
                                triangles.push_back(t.base);
                            }
                            convex_mesh_bvh_.insert(
                                aabb,
                                RigidBodyAndIntersectableMesh{
                                    .rb = rigid_body,
                                    .mesh = {
                                        .physics_material = m->physics_material,
                                        .mesh = std::make_shared<StaticTransformedMesh>(
                                            m->name,
                                            aabb,
                                            bounding_sphere,
                                            std::move(triangles),
                                            std::move(lines))}});
                        } else if (!any(m->physics_material & PhysicsMaterial::ATTR_CONCAVE)) {
                            THROW_OR_ABORT(
                                "Unknown physics material for terrain object \"" +
                                rigid_body->name() + "\" and mesh \"" + m->name +
                                "\" (neither obj_grind_line nor convex or concave)");
                        }
                        for (const auto& t : m->transformed_triangles_bbox(rigid_body->get_new_absolute_model_matrix())) {
                            triangle_bvh_.insert(t.aabb, {*rigid_body, t.base});
                        }
                    }
                }
            }
        };
        add_hitboxes(s_hitboxes);
        add_hitboxes(d_hitboxes);
        static_rigid_bodies_.push_back(rigid_body);
    } else {
        RigidBodyAndMeshes rbm;
        rbm.rigid_body = rigid_body;
        auto add_hitboxes = [&]<typename TPos>(
            const std::list<std::shared_ptr<ColoredVertexArray<TPos>>>& hitboxes,
            std::list<TypedMesh<std::pair<BoundingSphere<TPos, 3>, std::shared_ptr<ColoredVertexArray<TPos>>>>>& meshes)
        {
            for (auto& cva : hitboxes) {
                if (physics_resource_filter.matches(*cva)) {
                    if (any(cva->physics_material & PhysicsMaterial::OBJ_ALIGNMENT_PLANE)) {
                        THROW_OR_ABORT("Alignment planes only supported for terrain");
                    }
                    auto any_line_only_mask =
                        PhysicsMaterial::OBJ_TIRE_LINE |
                        PhysicsMaterial::OBJ_ALIGNMENT_CONTACT |
                        PhysicsMaterial::OBJ_BULLET_LINE_SEGMENT;
                    auto any_mesh_only_mask =
                        PhysicsMaterial::OBJ_GRIND_CONTACT |
                        PhysicsMaterial::OBJ_BULLET_MESH;
                    auto any_mask = PhysicsMaterial::OBJ_HITBOX;
                    if (any(cva->physics_material & any_line_only_mask)) {
                        assert_true(cva->triangles.empty());
                        assert_true(!cva->lines.empty());
                    } else if (any(cva->physics_material & any_mesh_only_mask)) {
                        assert_true(!cva->triangles.empty());
                        assert_true(cva->lines.empty());
                    } else if (any(cva->physics_material & any_mask)) {
                        // Do nothing
                    } else if (
                        any(cva->physics_material & PhysicsMaterial::ATTR_CONVEX) ==
                        any(cva->physics_material & PhysicsMaterial::ATTR_CONCAVE))
                    {
                        THROW_OR_ABORT(
                            "Physics material is not convex xor concave for movable object \"" +
                            rigid_body->name() + "\" and mesh \"" + cva->name +
                            "\" (neither obj_grind_line nor convex or concave)");
                    }
                    auto vertices = cva->vertices();
                    if (!vertices.empty()) {
                        BoundingSphere<TPos, 3> bs = welzl_from_iterator<TPos, 3>(vertices.begin(), vertices.end(), rng);
                        meshes.push_back({
                            .physics_material = cva->physics_material,
                            .mesh = std::make_pair(bs, cva)});
                    }
                }
            }
        };
        add_hitboxes(s_hitboxes, rbm.smeshes);
        add_hitboxes(d_hitboxes, rbm.dmeshes);
        if (collidable_mode == CollidableMode::SMALL_STATIC) {
            if (rigid_body->mass() != INFINITY) {
                THROW_OR_ABORT("Small static requires infinite mass");
            }
            transform_object_and_add(rbm);
        } else if (collidable_mode == CollidableMode::SMALL_MOVING) {
            if (!std::isfinite(rigid_body->mass())) {
                THROW_OR_ABORT("Small moving requires finite mass");
            }
            objects_.push_back(std::move(rbm));
        }
    }
    if (!collidable_modes_.insert({rigid_body.get(), collidable_mode}).second) {
        THROW_OR_ABORT("Could not insert collidable mode");
    }
    rigid_body->set_rigid_bodies(*this);
}

void RigidBodies::delete_rigid_body(const RigidBodyVehicle* rigid_body) {
    auto it = collidable_modes_.find(rigid_body);
    if (it == collidable_modes_.end()) {
        THROW_OR_ABORT("Could not find rigid body for deletion");
    }
    if (rigid_body->mass() == INFINITY) {
        if (it->second == CollidableMode::TERRAIN) {
            auto it = std::find_if(static_rigid_bodies_.begin(), static_rigid_bodies_.end(), [rigid_body](const auto& e){ return e.get() == rigid_body; });
            if (it == static_rigid_bodies_.end()) {
                THROW_OR_ABORT("Could not delete static rigid body (0)");
            }
            static_rigid_bodies_.erase(it);
            convex_mesh_bvh_.clear();
            triangle_bvh_.clear();
            line_bvh_.clear();
        } else if (it->second == CollidableMode::SMALL_STATIC)
        {
            auto it = std::find_if(transformed_objects_.begin(), transformed_objects_.end(), [rigid_body](const auto& e){ return e.rigid_body.get() == rigid_body; });
            if (it == transformed_objects_.end()) {
                THROW_OR_ABORT("Could not delete static rigid body (1)");
            }
            transformed_objects_.erase(it);
        } else {
            THROW_OR_ABORT("Could not delete rigid body (3)");
        }
    } else if (it->second == CollidableMode::SMALL_MOVING) {
        {
            auto it = std::find_if(objects_.begin(), objects_.end(), [rigid_body](const auto& e){ return e.rigid_body.get() == rigid_body; });
            if (it == objects_.end()) {
                THROW_OR_ABORT("Could not delete dynamic rigid body (4)");
            }
            objects_.erase(it);
        }
        transformed_objects_.remove_if([rigid_body](const RigidBodyAndIntersectableMeshes& rbtm){
            return (rbtm.rigid_body.get() == rigid_body);
        });
    } else {
        THROW_OR_ABORT("Could not delete rigid body (5)");
    }
    collidable_modes_.erase(it);
}

void RigidBodies::transform_object_and_add(const RigidBodyAndMeshes& o) {
    auto m = o.rigid_body->get_new_absolute_model_matrix();
    std::list<TypedMesh<std::shared_ptr<IntersectableMesh>>> transformed_meshes;
    auto add_meshes = [&](const auto& meshes){
        for (const auto& msh : meshes) {
            transformed_meshes.push_back({
                .physics_material = msh.physics_material,
                .mesh = std::make_shared<LazyTransformedMesh>(
                    m,
                    msh.mesh.first,
                    msh.mesh.second)});
        }
    };
    add_meshes(o.smeshes);
    add_meshes(o.dmeshes);
    transformed_objects_.push_back({
        .rigid_body = o.rigid_body,
        .meshes = std::move(transformed_meshes)});
}

void RigidBodies::optimize_search_time(std::ostream& ostr) const {
    convex_mesh_bvh_.optimize_search_time(ostr);
    triangle_bvh_.optimize_search_time(ostr);
    line_bvh_.optimize_search_time(ostr);
}

void RigidBodies::print_search_time() const {
    std::cout << "Convex mesh search time: " << convex_mesh_bvh_.search_time() << std::endl;
    std::cout << "Triangle search time: " << triangle_bvh_.search_time() << std::endl;
    std::cout << "Line search time: " << line_bvh_.search_time() << std::endl;
}

void RigidBodies::plot_convex_mesh_bvh_svg(const std::string& filename, size_t axis0, size_t axis1) const {
    convex_mesh_bvh_.plot_svg<double>(filename, axis0, axis1);
}

void RigidBodies::plot_triangle_bvh_svg(const std::string& filename, size_t axis0, size_t axis1) const {
    triangle_bvh_.plot_svg<double>(filename, axis0, axis1);
}

void RigidBodies::plot_line_bvh_svg(const std::string& filename, size_t axis0, size_t axis1) const {
    line_bvh_.plot_svg<double>(filename, axis0, axis1);
}

Iterable<std::list<RigidBodyAndMeshes>> RigidBodies::objects() const {
    return Iterable<std::list<RigidBodyAndMeshes>>(
        const_cast<std::list<RigidBodyAndMeshes>&>(objects_));
}

Iterable<std::list<RigidBodyAndIntersectableMeshes>> RigidBodies::transformed_objects() const {
    return Iterable<std::list<RigidBodyAndIntersectableMeshes>>(
        const_cast<std::list<RigidBodyAndIntersectableMeshes>&>(transformed_objects_));
}

const Bvh<double, RigidBodyAndIntersectableMesh, 3>& RigidBodies::convex_mesh_bvh() const {
    return convex_mesh_bvh_;
}

const Bvh<double, RigidBodyAndCollisionTriangleSphere, 3>& RigidBodies::triangle_bvh() const {
    return triangle_bvh_;
}

const Bvh<double, RigidBodyAndCollisionLineSphere, 3>& RigidBodies::line_bvh() const {
    return line_bvh_;
}
