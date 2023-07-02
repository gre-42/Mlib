#include "Rigid_Bodies.hpp"
#include <Mlib/Assert.hpp>
#include <Mlib/Geometry/Colored_Vertex.hpp>
#include <Mlib/Geometry/Intersection/Welzl.hpp>
#include <Mlib/Geometry/Mesh/Collision_Edges.hpp>
#include <Mlib/Geometry/Mesh/Collision_Ridges.hpp>
#include <Mlib/Geometry/Mesh/Colored_Vertex_Array.hpp>
#include <Mlib/Geometry/Mesh/Lazy_Transformed_Mesh.hpp>
#include <Mlib/Geometry/Mesh/Static_Transformed_Mesh.hpp>
#include <Mlib/Geometry/Physics_Material.hpp>
#include <Mlib/Images/Svg.hpp>
#include <Mlib/Physics/Collision/Collidable_Mode.hpp>
#include <Mlib/Physics/Physics_Engine/Physics_Engine_Config.hpp>
#include <Mlib/Physics/Rigid_Body/Rigid_Body_Vehicle.hpp>
#include <Mlib/Scene_Graph/Resources/Physics_Resource_Filter.hpp>
#include <Mlib/Throw_Or_Abort.hpp>

using namespace Mlib;

RigidBodies::RigidBodies(const PhysicsEngineConfig& cfg)
: cfg_{cfg},
  convex_mesh_bvh_{{cfg.bvh_max_size, cfg.bvh_max_size, cfg.bvh_max_size}, cfg.bvh_levels},
  triangle_bvh_{{cfg.bvh_max_size, cfg.bvh_max_size, cfg.bvh_max_size}, cfg.bvh_levels},
  ridge_bvh_{{cfg.bvh_max_size, cfg.bvh_max_size, cfg.bvh_max_size}, cfg.bvh_levels},
  line_bvh_{{cfg.bvh_max_size, cfg.bvh_max_size, cfg.bvh_max_size}, cfg.bvh_levels},
  collision_ridges_baking_status_{CollisionRidgeBakingStatus::NOT_BAKED}
{}

RigidBodies::~RigidBodies() = default;

void RigidBodies::add_rigid_body(
    std::unique_ptr<RigidBodyVehicle>&& rigid_body,
    const std::list<std::shared_ptr<ColoredVertexArray<float>>>& s_hitboxes,
    const std::list<std::shared_ptr<ColoredVertexArray<double>>>& d_hitboxes,
    CollidableMode collidable_mode,
    const PhysicsResourceFilter& physics_resource_filter,
    CollisionRidgeErrorBehavior collision_ridge_error_behavior)
{
    auto& rb = *rigid_body;
    if (!rigid_bodies_.try_emplace(rigid_body.get(), std::move(rigid_body)).second) {
        THROW_OR_ABORT("Rigid body already exists");
    }
    if (!collidable_modes_.insert({&rb, collidable_mode}).second) {
        verbose_abort("Could not insert collidable mode");
    }
    rb.set_rigid_bodies(*this);
    auto rng = welzl_rng();
    if (collidable_mode == CollidableMode::STATIC) {
        if (rb.mass() != INFINITY) {
            THROW_OR_ABORT("Terrain requires infinite mass");
        }
        // if (!tirelines.empty()) {
        //     THROW_OR_ABORT("static rigid body has tirelines");
        // }
        auto add_hitboxes = [&]<typename TPos>(const std::list<std::shared_ptr<ColoredVertexArray<TPos>>>& hitboxes) {
            for (auto& m : hitboxes) {
                if (physics_resource_filter.matches(*m)) {
                    if (any(m->physics_material & PhysicsMaterial::OBJ_GRIND_LINE)) {
                        for (const auto& t : m->transformed_lines_bbox(rb.get_new_absolute_model_matrix())) {
                            line_bvh_.insert(t.aabb, {rb, t.base});
                        }
                    } else {
                        bool is_convex = any(m->physics_material & PhysicsMaterial::ATTR_CONVEX);
                        bool is_concave = any(m->physics_material & PhysicsMaterial::ATTR_CONCAVE);
                        if (is_convex == is_concave) {
                            THROW_OR_ABORT(
                                "Physics material is neither obj_grind_line, nor convex xor concave. Object: \"" + rb.name() +
                                "\", mesh \"" + m->name +
                                " convex: " + std::to_string(int(is_convex)) +
                                ", concave: " + std::to_string(int(is_concave)));
                        }
                        if (any(m->physics_material & PhysicsMaterial::ATTR_CONVEX)) {
                            auto transformed = m->transformed_triangles_bbox(rb.get_new_absolute_model_matrix());
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
                            std::vector<CollisionLineSphere> edges;
                            std::vector<CollisionLineSphere> lines;
                            triangles.reserve(transformed.size());
                            for (const CollisionTriangleAabb& t : transformed) {
                                triangles.push_back(t.base);
                            }

                            CollisionEdges collision_edges;
                            for (const auto& t : triangles) {
                                collision_edges.insert(t.triangle, t.physics_material);
                            }
                            edges.reserve(collision_edges.size());
                            for (const auto& e : collision_edges) {
                                edges.push_back(e.collision_line_sphere);
                            }

                            convex_mesh_bvh_.insert(
                                aabb,
                                RigidBodyAndIntersectableMesh{
                                    .rb = rb,
                                    .mesh = {
                                        .physics_material = m->physics_material,
                                        .mesh = std::make_shared<StaticTransformedMesh>(
                                            m->name,
                                            aabb,
                                            bounding_sphere,
                                            std::move(triangles),
                                            std::move(lines),
                                            std::move(edges),
                                            std::vector<CollisionRidgeSphere>{})}});
                        } else {
                            if (collision_ridges_baking_status_ != CollisionRidgeBakingStatus::NOT_BAKED) {
                                THROW_OR_ABORT("Collision ridges already baked, or previous baking failed");
                            }
                            auto transformed = m->transformed_triangles_bbox(rb.get_new_absolute_model_matrix());
                            for (const auto& t : transformed) {
                                triangle_bvh_.insert(t.aabb, {rb, t.base});
                            }
                            auto insert_triangle = [this, &rb, collision_ridge_error_behavior](
                                CollisionRidgesRigidBody& collision_ridges,
                                const CollisionTriangleAabb& t)
                            {
                                collision_ridges.insert(
                                    t.base.triangle,
                                    t.base.plane.normal,
                                    cfg_.max_min_cos_ridge,
                                    t.base.physics_material,
                                    rb,
                                    collision_ridge_error_behavior);
                            };
                            if (any(m->physics_material & PhysicsMaterial::ATTR_SELF_CONTAINED)) {
                                CollisionRidgesRigidBody collision_ridges;
                                for (const auto& t : transformed) {
                                    insert_triangle(collision_ridges, t);
                                }
                                bake_collision_ridges(collision_ridges);
                            } else {
                                for (const auto& t : transformed) {
                                    insert_triangle(global_collision_ridges_, t);
                                }
                            }
                        }
                    }
                }
            }
        };
        static_rigid_bodies_.push_back(&rb);
        add_hitboxes(s_hitboxes);
        add_hitboxes(d_hitboxes);
    } else if (collidable_mode == CollidableMode::MOVING) {
        if (!std::isfinite(rb.mass())) {
            THROW_OR_ABORT("Moving object requires finite mass");
        }
        RigidBodyAndMeshes& rbm = objects_.emplace_back(RigidBodyAndMeshes{ .rigid_body = rb });
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
                            rb.name() + "\" and mesh \"" + cva->name +
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
    }
}

void RigidBodies::delete_rigid_body(const RigidBodyVehicle* rigid_body) {
    auto it = collidable_modes_.find(rigid_body);
    if (it == collidable_modes_.end()) {
        THROW_OR_ABORT("Could not find rigid body for deletion (collidable mode)");
    }
    if (rigid_body->mass() == INFINITY) {
        if (it->second == CollidableMode::STATIC) {
            auto it = std::find_if(static_rigid_bodies_.begin(), static_rigid_bodies_.end(), [rigid_body](const auto& e){ return e == rigid_body; });
            if (it == static_rigid_bodies_.end()) {
                THROW_OR_ABORT("Could not delete static rigid body (0)");
            }
            static_rigid_bodies_.erase(it);
            convex_mesh_bvh_.clear();
            triangle_bvh_.clear();
            ridge_bvh_.clear();
            line_bvh_.clear();
            global_collision_ridges_.clear();
            collision_ridges_baking_status_ = CollisionRidgeBakingStatus::NOT_BAKED;
        } else {
            THROW_OR_ABORT("Could not delete rigid body (3)");
        }
    } else if (it->second == CollidableMode::MOVING) {
        {
            auto it = std::find_if(objects_.begin(), objects_.end(), [rigid_body](const auto& e){ return &e.rigid_body == rigid_body; });
            if (it == objects_.end()) {
                THROW_OR_ABORT("Could not delete dynamic rigid body (4)");
            }
            objects_.erase(it);
        }
        transformed_objects_.remove_if([rigid_body](const RigidBodyAndIntersectableMeshes& rbtm){
            return (&rbtm.rigid_body == rigid_body);
        });
    } else {
        THROW_OR_ABORT("Could not delete rigid body (5)");
    }
    collidable_modes_.erase(it);
    rigid_bodies_.erase(rigid_body);
}

void RigidBodies::transform_object_and_add(const RigidBodyAndMeshes& o) {
    auto m = o.rigid_body.get_new_absolute_model_matrix();
    std::list<TypedMesh<std::shared_ptr<IIntersectableMesh>>> transformed_meshes;
    auto add_meshes = [&](const auto& meshes){
        for (const auto& msh : meshes) {
            transformed_meshes.push_back({
                .physics_material = msh.physics_material,
                .mesh = std::make_shared<LazyTransformedMesh>(
                    m,
                    msh.mesh.first,
                    msh.mesh.second,
                    cfg_.max_min_cos_ridge)});
        }
    };
    add_meshes(o.smeshes);
    add_meshes(o.dmeshes);
    transformed_objects_.push_back({
        .rigid_body = o.rigid_body,
        .meshes = std::move(transformed_meshes)});
}

void RigidBodies::optimize_search_time(std::ostream& ostr) const {
    convex_mesh_bvh_.optimize_search_time(BvhDataRadiusType::NONZERO, ostr);
    triangle_bvh_.optimize_search_time(BvhDataRadiusType::NONZERO, ostr);
    line_bvh_.optimize_search_time(BvhDataRadiusType::NONZERO, ostr);
}

void RigidBodies::print_search_time() const {
    std::cout << "Convex mesh search time: " << convex_mesh_bvh_.search_time(BvhDataRadiusType::NONZERO) << std::endl;
    std::cout << "Triangle search time: " << triangle_bvh_.search_time(BvhDataRadiusType::NONZERO) << std::endl;
    std::cout << "Line search time: " << line_bvh_.search_time(BvhDataRadiusType::NONZERO) << std::endl;
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

IterableWrapper<std::list<RigidBodyAndMeshes>> RigidBodies::objects() const {
    return IterableWrapper<std::list<RigidBodyAndMeshes>>(
        const_cast<std::list<RigidBodyAndMeshes>&>(objects_));
}

IterableWrapper<std::list<RigidBodyAndIntersectableMeshes>> RigidBodies::transformed_objects() const {
    return IterableWrapper<std::list<RigidBodyAndIntersectableMeshes>>(
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

const Bvh<double, RigidBodyAndCollisionRidgeSphere, 3>& RigidBodies::ridge_bvh() const {
    if (collision_ridges_baking_status_ == CollisionRidgeBakingStatus::BAKING) {
        THROW_OR_ABORT("Previous collision ridges baking failed");
    }
    if (collision_ridges_baking_status_ == CollisionRidgeBakingStatus::NOT_BAKED) {
        collision_ridges_baking_status_ = CollisionRidgeBakingStatus::BAKING;
        bake_collision_ridges(global_collision_ridges_);
        collision_ridges_baking_status_ = CollisionRidgeBakingStatus::BAKED;
    }
    return ridge_bvh_;
}

void RigidBodies::bake_collision_ridges(
    const CollisionRidgesRigidBody& collision_ridges) const
{
    for (const auto& e : collision_ridges) {
        ridge_bvh_.insert(
            AxisAlignedBoundingBox<double, 3>{e.collision_ridge_sphere.edge},
            RigidBodyAndCollisionRidgeSphere{
                .rb = e.rb,
                .crp = e.collision_ridge_sphere});
    }
}
