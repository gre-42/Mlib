#include "Rigid_Bodies.hpp"
#include <Mlib/Assert.hpp>
#include <Mlib/Geometry/Colored_Vertex.hpp>
#include <Mlib/Geometry/Interfaces/IIntersectable.hpp>
#include <Mlib/Geometry/Intersection/Welzl.hpp>
#include <Mlib/Geometry/Mesh/Collision_Edges.hpp>
#include <Mlib/Geometry/Mesh/Collision_Mesh.hpp>
#include <Mlib/Geometry/Mesh/Collision_Ridges.hpp>
#include <Mlib/Geometry/Mesh/Colored_Vertex_Array.hpp>
#include <Mlib/Geometry/Mesh/Lazy_Transformed_Mesh.hpp>
#include <Mlib/Geometry/Mesh/Static_Transformed_Mesh.hpp>
#include <Mlib/Geometry/Physics_Material.hpp>
#include <Mlib/Images/Svg.hpp>
#include <Mlib/Memory/Destruction_Functions_Removeal_Tokens_Object.hpp>
#include <Mlib/Physics/Collision/Collidable_Mode.hpp>
#include <Mlib/Physics/Physics_Engine/Physics_Engine_Config.hpp>
#include <Mlib/Physics/Rigid_Body/Rigid_Body_Vehicle.hpp>
#include <Mlib/Throw_Or_Abort.hpp>

using namespace Mlib;

RigidBodies::RigidBodies(const PhysicsEngineConfig& cfg)
    : cfg_{ cfg }
    , convex_mesh_bvh_{ {cfg.bvh_max_size, cfg.bvh_max_size, cfg.bvh_max_size}, cfg.bvh_levels }
    , triangle_bvh_{ {cfg.bvh_max_size, cfg.bvh_max_size, cfg.bvh_max_size}, cfg.bvh_levels }
    , ridge_bvh_{ {cfg.bvh_max_size, cfg.bvh_max_size, cfg.bvh_max_size}, cfg.bvh_levels }
    , line_bvh_{ {cfg.bvh_max_size, cfg.bvh_max_size, cfg.bvh_max_size}, cfg.bvh_levels }
    , collision_ridges_baking_status_{ CollisionRidgeBakingStatus::NOT_BAKED }
{}

RigidBodies::~RigidBodies() {
    bool success = true;
    if (!rigid_bodies_.empty()) {
        success = false;
        lerr() << "~RigidBodies: " << rigid_bodies_.size() << " rigid_bodies still exist.";
        for (const auto& [k, v] : rigid_bodies_) {
            lerr() << "  " << k->name();
        }
    }
    if (!objects_.empty()) {
        success = false;
        lerr() << "~RigidBodies: " << objects_.size() << " objects still exist.";
        for (const auto& o : objects_) {
            lerr() << "  " << o.rigid_body->name();
        }
    }
    if (!success) {
        verbose_abort("~RigidBodies contains dangling pointers");
    }
}

void RigidBodies::add_rigid_body(
    RigidBodyVehicle& rigid_body,
    const std::list<std::shared_ptr<ColoredVertexArray<float>>>& s_hitboxes,
    const std::list<std::shared_ptr<ColoredVertexArray<CompressedScenePos>>>& d_hitboxes,
    const std::list<TypedMesh<std::shared_ptr<IIntersectable>>>& intersectables,
    CollidableMode collidable_mode)
{
    auto& rb = rigid_body;
    bool has_meshes_or_intersectables = !s_hitboxes.empty() || !d_hitboxes.empty() || !intersectables.empty();
    if ((collidable_mode == CollidableMode::NONE) && has_meshes_or_intersectables) {
        THROW_OR_ABORT("Non-collidable has meshes or intersectables: \"" + rb.name() + '"');
    }
    if ((collidable_mode != CollidableMode::NONE) && !has_meshes_or_intersectables) {
        THROW_OR_ABORT("Collidable has no meshes or intersectables: \"" + rb.name() + '"');
    }
    {
        auto rit = rigid_bodies_.try_emplace(&rb, DanglingBaseClassRef<RigidBodyVehicle>{ rb, CURRENT_SOURCE_LOCATION }, CURRENT_SOURCE_LOCATION);
        if (!rit.second) {
            verbose_abort("Rigid body already exists");
        }
        rit.first->second.on_destroy([this, &rb]() { delete_rigid_body(rb); }, CURRENT_SOURCE_LOCATION);
    }
    if (!collidable_modes_.insert({ &rb, collidable_mode }).second) {
        verbose_abort("Could not insert collidable mode");
    }
    auto rng = welzl_rng();
    if (collidable_mode == CollidableMode::STATIC) {
        if (rb.mass() != INFINITY) {
            THROW_OR_ABORT("Terrain requires infinite mass");
        }
        if (!intersectables.empty()) {
            THROW_OR_ABORT("Intersectables only supported for moving objects");
        }
        // if (!tirelines.empty()) {
        //     THROW_OR_ABORT("static rigid body has tirelines");
        // }
        auto add_hitboxes = [&]<typename TPos>(const std::list<std::shared_ptr<ColoredVertexArray<TPos>>>& hitboxes) {
            for (auto& m : hitboxes) {
                if (any(m->morphology.physics_material & PhysicsMaterial::OBJ_GRIND_LINE)) {
                    for (const auto& t : m->transformed_lines_bbox(rb.get_new_absolute_model_matrix())) {
                        line_bvh_.insert(t.aabb, RigidBodyAndCollisionLineSphere<CompressedScenePos>{ rb, t.base });
                    }
                } else {
                    bool is_convex = any(m->morphology.physics_material & PhysicsMaterial::ATTR_CONVEX);
                    bool is_concave = any(m->morphology.physics_material & PhysicsMaterial::ATTR_CONCAVE);
                    if (is_convex == is_concave) {
                        THROW_OR_ABORT(
                            "Physics material is neither obj_grind_line, nor convex xor concave. Object: \"" + rb.name() +
                            "\", mesh \"" + m->name +
                            " convex: " + std::to_string(int(is_convex)) +
                            ", concave: " + std::to_string(int(is_concave)));
                    }
                    if (any(m->morphology.physics_material & PhysicsMaterial::ATTR_CONVEX)) {
                        CollisionRidges<CompressedScenePos> collision_ridges;
                        std::set<OrderableFixedArray<ScenePos, 3>> vertex_set;
                        std::vector<const FixedArray<ScenePos, 3>*> vertex_vector;
                        vertex_vector.reserve(4 * m->quads.size() + 3 * m->triangles.size());
                        auto get_transformed = [&]<size_t tnvertices>(){
                            auto transformed = m->template transformed_polygon_bbox<tnvertices>(
                                rb.get_new_absolute_model_matrix());
                            std::vector<CollisionPolygonSphere<CompressedScenePos, tnvertices>> bases;
                            bases.reserve(transformed.size());
                            for (const CollisionPolygonAabb<CompressedScenePos, tnvertices>& t : transformed) {
                                bases.push_back(t.base);
                            }
                            for (const auto& t : bases) {
                                for (const auto& v : t.corners.row_iterable()) {
                                    if (auto it = vertex_set.insert(OrderableFixedArray{v.template casted<ScenePos>()}); it.second) {
                                        vertex_vector.push_back(&*it.first);
                                    }
                                }
                                collision_ridges.insert(
                                    t.corners,
                                    t.polygon.plane.normal,
                                    cfg_.max_min_cos_ridge,
                                    t.physics_material);
                            }
                            return bases;
                        };
                        std::vector<CollisionPolygonSphere<CompressedScenePos, 4>> quads = get_transformed.template operator()<4>();
                        std::vector<CollisionPolygonSphere<CompressedScenePos, 3>> triangles = get_transformed.template operator()<3>();
                        std::vector<CollisionRidgeSphere<CompressedScenePos>> ridges;
                        std::vector<CollisionLineSphere<CompressedScenePos>> lines;
                        std::vector<TypedMesh<std::shared_ptr<IIntersectable>>> intersectables;

                        auto aabb =
                            AxisAlignedBoundingBox<ScenePos, 3>::from_iterator(vertex_set.begin(), vertex_set.end())
                            .casted<CompressedScenePos>();
                        BoundingSphere<CompressedScenePos, 3> bounding_sphere =
                            welzl_from_vector<ScenePos, 3>(vertex_vector, rng)
                            .casted<CompressedScenePos>();

                        ridges.reserve(collision_ridges.size());
                        for (const auto& e : collision_ridges) {
                            if (e.collision_ridge_sphere.is_touchable(SingleFaceBehavior::UNTOUCHABLE)) {
                                ridges.emplace_back(e.collision_ridge_sphere).finalize();
                            }
                        }

                        convex_mesh_bvh_.insert(
                            aabb,
                            RigidBodyAndIntersectableMesh{
                                .rb = { rb, CURRENT_SOURCE_LOCATION },
                                .mesh = {
                                    .physics_material = m->morphology.physics_material,
                                    .mesh = std::make_shared<StaticTransformedMesh>(
                                        m->name,
                                        aabb,
                                        bounding_sphere,
                                        std::move(quads),
                                        std::move(triangles),
                                        std::move(lines),
                                        std::vector<CollisionLineSphere<CompressedScenePos>>{},
                                        std::move(ridges),
                                        std::move(intersectables))}});
                    } else {
                        if (collision_ridges_baking_status_ != CollisionRidgeBakingStatus::NOT_BAKED) {
                            THROW_OR_ABORT("Collision ridges already baked, or previous baking failed");
                        }
                        auto add = [&]<size_t tnvertices>(){
                            auto transformed = m->template transformed_polygon_bbox<tnvertices>(
                                rb.get_new_absolute_model_matrix());
                            for (const auto& t : transformed) {
                                triangle_bvh_.insert(t.aabb, RigidBodyAndCollisionTriangleSphere<CompressedScenePos>{ rb, t.base });
                            }
                            if (collision_ridges_baking_status_ == CollisionRidgeBakingStatus::BAKED) {
                                THROW_OR_ABORT("Collision ridges already baked");
                            }
                            for (const auto& t : transformed) {
                                collision_ridges_.insert(
                                    t.base.corners,
                                    t.base.polygon.plane.normal,
                                    cfg_.max_min_cos_ridge,
                                    t.base.physics_material,
                                    rb);
                            }
                        };
                        // From: https://stackoverflow.com/a/77429864/2292832
                        add.template operator()<4>();
                        add.template operator()<3>();
                    }
                }
            }
        };
        add_hitboxes(s_hitboxes);
        add_hitboxes(d_hitboxes);
    } else if ((collidable_mode == CollidableMode::MOVING) ||
               (collidable_mode == CollidableMode::NONE))
    {
        if (!std::isfinite(rb.mass())) {
            THROW_OR_ABORT("Moving object requires finite mass");
        }
        RigidBodyAndMeshes& rbm = objects_.emplace_back(RigidBodyAndMeshes{ .rigid_body = { rb, CURRENT_SOURCE_LOCATION } });
        auto add_hitboxes = [&]<typename TPos>(
            const std::list<std::shared_ptr<ColoredVertexArray<TPos>>>& hitboxes,
            std::list<TypedMesh<std::pair<BoundingSphere<CompressedScenePos, 3>, std::shared_ptr<CollisionMesh>>>>& meshes)
        {
            for (auto& cva : hitboxes) {
                if (any(cva->morphology.physics_material & PhysicsMaterial::ATTR_CONCAVE)) {
                    THROW_OR_ABORT("Moving objects cannot be concave (due to ridge_map_)");
                }
                if (any(cva->morphology.physics_material & PhysicsMaterial::OBJ_ALIGNMENT_PLANE)) {
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
                if (any(cva->morphology.physics_material & any_line_only_mask)) {
                    assert_true(cva->triangles.empty());
                    assert_true(cva->quads.empty());
                    assert_true(!cva->lines.empty());
                } else if (any(cva->morphology.physics_material & any_mesh_only_mask)) {
                    assert_true(!cva->triangles.empty() || !cva->quads.empty());
                    assert_true(cva->lines.empty());
                } else if (any(cva->morphology.physics_material & any_mask)) {
                    // Do nothing
                } else if (
                    any(cva->morphology.physics_material & PhysicsMaterial::ATTR_CONVEX) ==
                    any(cva->morphology.physics_material & PhysicsMaterial::ATTR_CONCAVE))
                {
                    THROW_OR_ABORT(
                        "Physics material is not convex xor concave for movable object \"" +
                        rb.name() + "\" and mesh \"" + cva->name +
                        "\" (neither obj_grind_line nor convex or concave)");
                }
                auto vertices = cva->vertices();
                if (!vertices.empty()) {
                    BoundingSphere<CompressedScenePos, 3> bs =
                        welzl_from_iterator<TPos, 3>(vertices.begin(), vertices.end(), rng)
                        .template casted<CompressedScenePos>();
                    meshes.push_back({
                        .physics_material = cva->morphology.physics_material,
                        .mesh = std::make_pair(bs, std::make_shared<CollisionMesh>(*cva))});
                }
            }
        };
        add_hitboxes(s_hitboxes, rbm.meshes);
        add_hitboxes(d_hitboxes, rbm.meshes);
        for (const auto& intersectable : intersectables) {
            rbm.meshes.push_back({
                .physics_material = intersectable.physics_material,
                .mesh = std::make_pair(
                    intersectable.mesh->bounding_sphere(),
                    std::make_shared<CollisionMesh>(
                        "intersectable mesh",
                        intersectable))});
        }
    } else {
        THROW_OR_ABORT("Unknown collidable mode");
    }
}

void RigidBodies::delete_rigid_body(const RigidBodyVehicle& rigid_body) {
    auto it = collidable_modes_.find(&rigid_body);
    if (it == collidable_modes_.end()) {
        THROW_OR_ABORT("Could not find rigid body for deletion (collidable mode)");
    }
    if (rigid_body.mass() == INFINITY) {
        if (it->second == CollidableMode::STATIC) {
            convex_mesh_bvh_.clear();
            triangle_bvh_.clear();
            ridge_bvh_.clear();
            ridge_map_.clear();
            line_bvh_.clear();
            collision_ridges_.clear();
            collision_ridges_baking_status_ = CollisionRidgeBakingStatus::NOT_BAKED;
        } else {
            THROW_OR_ABORT("Could not delete rigid body (3)");
        }
    } else if ((it->second == CollidableMode::MOVING) ||
               (it->second == CollidableMode::NONE))
    {
        {
            auto it = std::find_if(objects_.begin(), objects_.end(), [&rigid_body](const auto& e){ return &e.rigid_body.get() == &rigid_body; });
            if (it == objects_.end()) {
                THROW_OR_ABORT("Could not delete dynamic rigid body (4)");
            }
            objects_.erase(it);
        }
        transformed_objects_.remove_if([&rigid_body](const RigidBodyAndIntersectableMeshes& rbtm){
            return (&rbtm.rigid_body.get() == &rigid_body);
        });
    } else {
        THROW_OR_ABORT("Could not delete rigid body (5)");
    }
    collidable_modes_.erase(it);
    rigid_bodies_.erase(&rigid_body);
}

void RigidBodies::transform_object_and_add(const RigidBodyAndMeshes& o) {
    if (!o.has_meshes()) {
        THROW_OR_ABORT("Attempt to add rigid body \"" + o.rigid_body->name() + "\" without meshes");
    }
    auto m = o.rigid_body->get_new_absolute_model_matrix();
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
    add_meshes(o.meshes);
    transformed_objects_.push_back({
        .rigid_body = o.rigid_body,
        .meshes = std::move(transformed_meshes) });
}

void RigidBodies::optimize_search_time(std::ostream& ostr) const {
    ostr << "Convex mesh BVH optimization" << std::endl;
    convex_mesh_bvh_.optimize_search_time(BvhDataRadiusType::NONZERO, ostr);
    ostr << "Triangle BVH optimization" << std::endl;
    triangle_bvh_.optimize_search_time(BvhDataRadiusType::NONZERO, ostr);
    ostr << "Ridge BVH optimization" << std::endl;
    ridge_bvh_.optimize_search_time(BvhDataRadiusType::NONZERO, ostr);
    ostr << "Line BVH optimization" << std::endl;
    line_bvh_.optimize_search_time(BvhDataRadiusType::NONZERO, ostr);
}

void RigidBodies::print_search_time() const {
    linfo() << "Convex mesh search time: " << convex_mesh_bvh_.search_time(BvhDataRadiusType::NONZERO);
    linfo() << "Triangle search time: " << triangle_bvh_.search_time(BvhDataRadiusType::NONZERO);
    linfo() << "Ridge search time: " << ridge_bvh_.search_time(BvhDataRadiusType::NONZERO);
    linfo() << "Line search time: " << line_bvh_.search_time(BvhDataRadiusType::NONZERO);
}

void RigidBodies::print_compression_ratio() const {
    size_t nsmall = 0;
    size_t nlarge = 0;
    triangle_bvh_.visit_bvhs([&](const auto& bvh){
        nsmall += bvh.data().small_size();
        nlarge += bvh.data().large_size();
        return true;
    });
    linfo() << "nsmall: " << nsmall;
    linfo() << "nlarge: " << nlarge;
    if (nsmall + nlarge > 0) {
        linfo() << "nsmall / ntotal: " <<
            100. * (integral_to_float<double>(nsmall) /
                    integral_to_float<double>(nsmall + nlarge)) << '%';
    }
}

void RigidBodies::plot_convex_mesh_bvh_svg(const std::string& filename, size_t axis0, size_t axis1) const {
    convex_mesh_bvh_.plot_svg<ScenePos>(filename, axis0, axis1);
}

void RigidBodies::plot_triangle_bvh_svg(const std::string& filename, size_t axis0, size_t axis1) const {
    triangle_bvh_.plot_svg<ScenePos>(filename, axis0, axis1);
}

void RigidBodies::plot_line_bvh_svg(const std::string& filename, size_t axis0, size_t axis1) const {
    line_bvh_.plot_svg<ScenePos>(filename, axis0, axis1);
}

IterableWrapper<std::list<RigidBodyAndMeshes>> RigidBodies::objects() const {
    return IterableWrapper<std::list<RigidBodyAndMeshes>>(
        const_cast<std::list<RigidBodyAndMeshes>&>(objects_));
}

IterableWrapper<std::list<RigidBodyAndIntersectableMeshes>> RigidBodies::transformed_objects() const {
    return IterableWrapper<std::list<RigidBodyAndIntersectableMeshes>>(
        const_cast<std::list<RigidBodyAndIntersectableMeshes>&>(transformed_objects_));
}

const Bvh<CompressedScenePos, 3, RigidBodyAndIntersectableMesh>& RigidBodies::convex_mesh_bvh() const {
    return convex_mesh_bvh_;
}

const RigidBodies::TriangleBvh& RigidBodies::triangle_bvh() const {
    return triangle_bvh_;
}

const RigidBodies::LineBvh& RigidBodies::line_bvh() const {
    return line_bvh_;
}

void RigidBodies::bake_collision_ridges_if_necessary() const {
    if (collision_ridges_baking_status_ == CollisionRidgeBakingStatus::BAKING) {
        THROW_OR_ABORT("Previous collision ridges baking failed");
    }
    if (collision_ridges_baking_status_ == CollisionRidgeBakingStatus::NOT_BAKED) {
        collision_ridges_baking_status_ = CollisionRidgeBakingStatus::BAKING;
        bake_collision_ridges();
        collision_ridges_baking_status_ = CollisionRidgeBakingStatus::BAKED;
    }
}

const RigidBodies::RidgeBvh& RigidBodies::ridge_bvh() const {
    bake_collision_ridges_if_necessary();
    return ridge_bvh_;
}

RidgeMap& RigidBodies::ridge_map() {
    bake_collision_ridges_if_necessary();
    return ridge_map_;
}

void RigidBodies::bake_collision_ridges() const
{
    while (!collision_ridges_.empty()) {
        auto node = collision_ridges_.extract(collision_ridges_.begin());
        const auto& e = node.value();
        if (!e.collision_ridge_sphere.is_touchable(SingleFaceBehavior::UNTOUCHABLE)) {
            continue;
        }
        RigidBodyAndCollisionRidgeSphere<CompressedScenePos> rcs{
            .rb = e.rb,
            .crp = e.collision_ridge_sphere};
        rcs.crp.finalize();
        ridge_bvh_.insert(
            AxisAlignedBoundingBox<CompressedScenePos, 3>::from_points(e.collision_ridge_sphere.edge),
            rcs);
        if (cfg_.enable_ridge_map) {
            auto a = OrderableFixedArray{ e.collision_ridge_sphere.edge[0] };
            auto b = OrderableFixedArray{ e.collision_ridge_sphere.edge[1] };
            if (a < b) {
                if (auto it = ridge_map_.try_emplace({a, b}, rcs); !it.second) {
                    std::stringstream sstr;
                    sstr << "Could not insert into ridge-map. Edge: " << a << " <-> " << b << "; Rigid bodies: \"" << e.rb.name() << "\", \"" << it.first->second.rb.name() << '"';
                    THROW_OR_ABORT(sstr.str());
                }
            } else {
                if (auto it = ridge_map_.try_emplace({b, a}, rcs); !it.second) {
                    std::stringstream sstr;
                    sstr << "Could not insert into ridge-map. Edge: " << b << " <-> " << a << "; Rigid bodies: \"" << e.rb.name() << "\", \"" << it.first->second.rb.name() << '"';
                    THROW_OR_ABORT(sstr.str());
                }
            }
        }
    }
}

bool RigidBodies::empty() const {
    return objects_.empty();
}
