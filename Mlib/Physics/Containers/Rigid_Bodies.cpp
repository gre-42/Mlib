
#include "Rigid_Bodies.hpp"
#include <Mlib/Geometry/Colored_Vertex.hpp>
#include <Mlib/Geometry/Graph/Cluster_By_Flood_Fill.hpp>
#include <Mlib/Geometry/Interfaces/IIntersectable.hpp>
#include <Mlib/Geometry/Mesh/Collision_Mesh.hpp>
#include <Mlib/Geometry/Mesh/Colored_Vertex_Array.hpp>
#include <Mlib/Geometry/Mesh/Lazy_Transformed_Mesh.hpp>
#include <Mlib/Geometry/Mesh/Static_Transformed_Mesh.hpp>
#include <Mlib/Geometry/Physics_Material.hpp>
#include <Mlib/Geometry/Welzl.hpp>
#include <Mlib/Images/Svg.hpp>
#include <Mlib/Math/Power_Of_Two_Divider.hpp>
#include <Mlib/Memory/Destruction_Functions_Removeal_Tokens_Ref.hpp>
#include <Mlib/Memory/Float_To_Integral.hpp>
#include <Mlib/Physics/Collision/Collidable_Mode.hpp>
#include <Mlib/Physics/Containers/Collision_Group.hpp>
#include <Mlib/Physics/Rigid_Body/Rigid_Body_Vehicle.hpp>
#include <Mlib/Scene_Config/Physics_Engine_Config.hpp>
#include <Mlib/Testing/Assert.hpp>
#include <stdexcept>

using namespace Mlib;

RigidBodies::RigidBodies(const PhysicsEngineConfig& cfg)
    : cfg_{ cfg }
    , is_colliding_{ false }
    , convex_mesh_bvh_{
        {cfg.bvh_max_size, cfg.bvh_max_size, cfg.bvh_max_size},
        cfg.bvh_levels,
        cfg.grid_level,
        cfg.ncells,
        fixed_full<CompressedScenePos, 3>(cfg.dilation_radius)
    }
    , triangle_bvh_{
        {cfg.bvh_max_size, cfg.bvh_max_size, cfg.bvh_max_size},
        cfg.bvh_levels,
        cfg.grid_level,
        cfg.ncells,
        fixed_full<CompressedScenePos, 3>(cfg.dilation_radius)
    }
    , line_bvh_{ {cfg.bvh_max_size, cfg.bvh_max_size, cfg.bvh_max_size}, cfg.bvh_levels }
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
    if (is_colliding_ && any(collidable_mode & CollidableMode::COLLIDE)) {
        throw std::runtime_error("Attempt to add rigid body during collision-phase (0)");
    }
    auto& rb = rigid_body;
    bool has_meshes_or_intersectables = !s_hitboxes.empty() || !d_hitboxes.empty() || !intersectables.empty();
    if (!any(collidable_mode & CollidableMode::COLLIDE) && has_meshes_or_intersectables) {
        throw std::runtime_error("Non-collidable has meshes or intersectables: \"" + rb.name() + '"');
    }
    if (any(collidable_mode & CollidableMode::COLLIDE) && !has_meshes_or_intersectables) {
        throw std::runtime_error("Collidable has no meshes or intersectables: \"" + rb.name() + '"');
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
    if (collidable_mode == CollidableMode::COLLIDE) {
        if (rb.mass() != INFINITY) {
            throw std::runtime_error("Terrain requires infinite mass");
        }
        if (!intersectables.empty()) {
            throw std::runtime_error("Intersectables only supported for moving objects");
        }
        // if (!tirelines.empty()) {
        //     throw std::runtime_error("static rigid body has tirelines");
        // }
        auto add_hitboxes = [&]<typename TPos>(const std::list<std::shared_ptr<ColoredVertexArray<TPos>>>& hitboxes) {
            for (auto& m : hitboxes) {
                auto add_to_line_bvh = [&](){
                    for (const auto& t : m->transformed_lines_bbox(rb.get_new_absolute_model_matrix())) {
                        line_bvh_.insert(t.aabb, RigidBodyAndCollisionLineSphere<CompressedScenePos>{ rb, t.base });
                    }
                };
                if (any(m->meta.morphology.physics_material & PhysicsMaterial::OBJ_GRIND_LINE)) {
                    if (!m->quads.empty()) {
                        throw std::runtime_error("Grind line has quads");
                    }
                    if (!m->triangles.empty()) {
                        throw std::runtime_error("Grind line has triangles");
                    }
                    add_to_line_bvh();
                } else {
                    bool is_convex = any(m->meta.morphology.physics_material & PhysicsMaterial::ATTR_CONVEX);
                    bool is_concave = any(m->meta.morphology.physics_material & PhysicsMaterial::ATTR_CONCAVE);
                    if (is_convex == is_concave) {
                        throw std::runtime_error(
                            "Physics material is neither obj_grind_line, nor convex xor concave. Object: \"" + rb.name() +
                            "\", mesh \"" + m->meta.name.full_name() +
                            " convex: " + std::to_string(int(is_convex)) +
                            ", concave: " + std::to_string(int(is_concave)));
                    }
                    if (any(m->meta.morphology.physics_material & PhysicsMaterial::ATTR_CONVEX)) {
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
                                    if (auto it = vertex_set.insert(make_orderable(v.template casted<ScenePos>())); it.second) {
                                        vertex_vector.push_back(&*it.first);
                                    }
                                }
                            }
                            return bases;
                        };
                        std::vector<CollisionPolygonSphere<CompressedScenePos, 4>> quads = get_transformed.template operator()<4>();
                        std::vector<CollisionPolygonSphere<CompressedScenePos, 3>> triangles = get_transformed.template operator()<3>();
                        std::vector<CollisionLineSphere<CompressedScenePos>> lines;
                        std::vector<TypedMesh<std::shared_ptr<IIntersectable>>> intersectables;

                        auto aabb =
                            AxisAlignedBoundingBox<ScenePos, 3>::from_iterator(vertex_set.begin(), vertex_set.end())
                            .casted<CompressedScenePos>();
                        BoundingSphere<CompressedScenePos, 3> bounding_sphere =
                            welzl_from_vector<ScenePos, 3>(vertex_vector, rng)
                            .casted<CompressedScenePos>();

                        auto lines_bbox = m->transformed_lines_bbox(rb.get_new_absolute_model_matrix());
                        lines.reserve(lines_bbox.size());
                        for (const auto& e : lines_bbox) {
                            lines.emplace_back(e.base);
                        }

                        convex_mesh_bvh_.root_bvh.insert(
                            aabb,
                            RigidBodyAndIntersectableMesh{
                                .rb = { rb, CURRENT_SOURCE_LOCATION },
                                .mesh = {
                                    .physics_material = m->meta.morphology.physics_material,
                                    .mesh = std::make_shared<StaticTransformedMesh>(
                                        m->meta.name.full_name(),
                                        aabb,
                                        bounding_sphere,
                                        std::move(quads),
                                        std::move(triangles),
                                        std::move(lines),
                                        std::vector<CollisionLineSphere<CompressedScenePos>>{},
                                        std::move(intersectables))}});
                    } else {
                        auto add = [&]<size_t tnvertices>(){
                            auto transformed = m->template transformed_polygon_bbox<tnvertices>(
                                rb.get_new_absolute_model_matrix());
                            for (const auto& t : transformed) {
                                triangle_bvh_.root_bvh.insert(t.aabb, RigidBodyAndCollisionTriangleSphere<CompressedScenePos>{ rb, t.base });
                            }
                        };
                        // From: https://stackoverflow.com/a/77429864/2292832
                        add.template operator()<4>();
                        add.template operator()<3>();
                        add_to_line_bvh();
                    }
                }
            }
        };
        add_hitboxes(s_hitboxes);
        add_hitboxes(d_hitboxes);
    } else if ((collidable_mode == (CollidableMode::COLLIDE | CollidableMode::MOVE)) ||
               (collidable_mode == CollidableMode::MOVE) ||
               (collidable_mode == CollidableMode::NONE))
    {
        if (!std::isfinite(rb.mass())) {
            throw std::runtime_error("Moving object requires finite mass");
        }
        RigidBodyAndMeshes& rbm = objects_.emplace_back(RigidBodyAndMeshes{ .rigid_body = { rb, CURRENT_SOURCE_LOCATION } });
        auto add_hitboxes = [&]<typename TPos>(
            const std::list<std::shared_ptr<ColoredVertexArray<TPos>>>& hitboxes,
            std::list<TypedMesh<std::pair<BoundingSphere<CompressedScenePos, 3>, std::shared_ptr<CollisionMesh>>>>& meshes)
        {
            for (auto& cva : hitboxes) {
                if (any(cva->meta.morphology.physics_material & PhysicsMaterial::ATTR_CONCAVE)) {
                    throw std::runtime_error("Moving objects cannot be concave (due to ridge_map_)");
                }
                if (any(cva->meta.morphology.physics_material & PhysicsMaterial::OBJ_ALIGNMENT_PLANE)) {
                    throw std::runtime_error("Alignment planes only supported for terrain");
                }
                auto any_line_only_mask =
                    PhysicsMaterial::OBJ_TIRE_LINE |
                    PhysicsMaterial::OBJ_ALIGNMENT_CONTACT |
                    PhysicsMaterial::OBJ_BULLET_LINE_SEGMENT;
                auto any_mesh_only_mask =
                    PhysicsMaterial::OBJ_GRIND_CONTACT |
                    PhysicsMaterial::OBJ_BULLET_MESH;
                auto any_mask = PhysicsMaterial::OBJ_HITBOX;
                if (any(cva->meta.morphology.physics_material & any_line_only_mask)) {
                    assert_true(cva->triangles.empty());
                    assert_true(cva->quads.empty());
                    assert_true(!cva->lines.empty());
                } else if (any(cva->meta.morphology.physics_material & any_mesh_only_mask)) {
                    assert_true(!cva->triangles.empty() || !cva->quads.empty());
                    assert_true(cva->lines.empty());
                } else if (any(cva->meta.morphology.physics_material & any_mask)) {
                    // Do nothing
                } else if (
                    any(cva->meta.morphology.physics_material & PhysicsMaterial::ATTR_CONVEX) ==
                    any(cva->meta.morphology.physics_material & PhysicsMaterial::ATTR_CONCAVE))
                {
                    throw std::runtime_error(
                        "Physics material is not convex xor concave for movable object \"" +
                        rb.name() + "\" and mesh \"" + cva->meta.name.full_name() +
                        "\" (neither obj_grind_line nor convex or concave)");
                }
                auto vertices = cva->vertices();
                if (!vertices.empty()) {
                    BoundingSphere<CompressedScenePos, 3> bs =
                        welzl_from_iterator<TPos, 3>(vertices.begin(), vertices.end(), rng)
                        .template casted<CompressedScenePos>();
                    meshes.push_back({
                        .physics_material = cva->meta.morphology.physics_material,
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
        if (rbm.has_meshes()) {
            transform_object_and_add(rbm);
        }
    } else {
        throw std::runtime_error("Unknown collidable mode");
    }
}

void RigidBodies::delete_rigid_body(const RigidBodyVehicle& rigid_body) {
    auto it = collidable_modes_.find(&rigid_body);
    if (it == collidable_modes_.end()) {
        throw std::runtime_error("Could not find rigid body for deletion (collidable mode)");
    }
    if (rigid_body.mass() == INFINITY) {
        if (it->second == CollidableMode::COLLIDE) {
            convex_mesh_bvh_.clear();
            triangle_bvh_.clear();
            line_bvh_.clear();
        } else {
            throw std::runtime_error("Could not delete rigid body (3)");
        }
    } else if ((it->second == (CollidableMode::COLLIDE | CollidableMode::MOVE)) ||
               (it->second == CollidableMode::MOVE) ||
               (it->second == CollidableMode::NONE))
    {
        {
            auto it2 = std::find_if(objects_.begin(), objects_.end(), [&rigid_body](const auto& e){ return &e.rigid_body.get() == &rigid_body; });
            if (it2 == objects_.end()) {
                throw std::runtime_error("Could not delete dynamic rigid body (4)");
            }
            objects_.erase(it2);
        }
        transformed_objects_.remove_if([&rigid_body](const RigidBodyAndIntersectableMeshes& rbtm){
            return (&rbtm.rigid_body.get() == &rigid_body);
        });
    } else {
        throw std::runtime_error("Could not delete rigid body (5)");
    }
    collidable_modes_.erase(it);
    rigid_bodies_.erase(&rigid_body);
}

void RigidBodies::transform_object_and_add(const RigidBodyAndMeshes& o) {
    if (!o.has_meshes()) {
        throw std::runtime_error("Attempt to add rigid body \"" + o.rigid_body->name() + "\" without meshes");
    }
    if (is_colliding_) {
        throw std::runtime_error("Attempt to add rigid body during collision-phase (1)");
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
                    msh.mesh.second)});
        }
    };
    add_meshes(o.meshes);
    transformed_objects_.push_back({
        .rigid_body = o.rigid_body,
        .meshes = std::move(transformed_meshes) });
}

void RigidBodies::optimize_search_time(std::ostream& ostr) const {
    ostr << "Convex mesh BVH optimization" << std::endl;
    convex_mesh_bvh_.root_bvh.optimize_search_time(BvhDataRadiusType::NONZERO, ostr);
    ostr << "Triangle BVH optimization" << std::endl;
    triangle_bvh_.root_bvh.optimize_search_time(BvhDataRadiusType::NONZERO, ostr);
    ostr << "Line BVH optimization" << std::endl;
    line_bvh_.optimize_search_time(BvhDataRadiusType::NONZERO, ostr);
}

void RigidBodies::print_search_time() const {
    linfo() << "Convex mesh search time: " << convex_mesh_bvh_.root_bvh.search_time(BvhDataRadiusType::NONZERO);
    linfo() << "Triangle search time: " << triangle_bvh_.root_bvh.search_time(BvhDataRadiusType::NONZERO);
    linfo() << "Line search time: " << line_bvh_.search_time(BvhDataRadiusType::NONZERO);
}

void RigidBodies::print_compression_ratio() const {
    size_t nsmall = 0;
    size_t nlarge = 0;
    triangle_bvh_.root_bvh.visit_bvhs([&](const auto& bvh){
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
    convex_mesh_bvh_.root_bvh.plot_svg<ScenePos>(filename, axis0, axis1);
}

void RigidBodies::plot_triangle_bvh_svg(const std::string& filename, size_t axis0, size_t axis1) const {
    triangle_bvh_.root_bvh.plot_svg<ScenePos>(filename, axis0, axis1);
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

const RigidBodies::ConvexMeshBvh& RigidBodies::convex_mesh_bvh() const {
    return convex_mesh_bvh_;
}

const RigidBodies::TriangleBvh& RigidBodies::triangle_bvh() const {
    return triangle_bvh_;
}

const RigidBodies::LineBvh& RigidBodies::line_bvh() const {
    return line_bvh_;
}

bool RigidBodies::empty() const {
    return objects_.empty();
}

struct RigidBodyAndMesh {
    RigidBodyVehicle& rb;
    RigidBodyAndMeshes::Mesh& mesh;
    BoundingSphere<CompressedScenePos, 3> mesh_sphere;
};

std::vector<CollisionGroup> RigidBodies::collision_groups() {
    std::list<RigidBodyAndMesh> standard_movables;
    std::unordered_set<RigidBodyPulses*> bullet_line_segments;
    for (auto& m : objects_) {
        if ((m.rigid_body->mass() != INFINITY) && !m.rigid_body->is_deactivated_avatar()) {
            for (auto& mesh : m.meshes) {
                // Bullet line segments are artificially long,
                // so they work without substepping.
                if (any(mesh.physics_material & PhysicsMaterial::OBJ_BULLET_LINE_SEGMENT)) {
                    bullet_line_segments.insert(&m.rigid_body->rbp_);
                } else {
                    standard_movables.emplace_back(
                        m.rigid_body.get(),
                        mesh,
                        mesh.mesh.first.transformed(m.rigid_body->rbp_.abs_transformation()));
                }
            }
        }
    }
    auto clusters = cluster_by_flood_fill(
        standard_movables,
        [this](RigidBodyAndMesh& a, RigidBodyAndMesh& b){
            return
                a.mesh_sphere.intersects(b.mesh_sphere, (CompressedScenePos)cfg_.max_penetration) ||
                a.rb.collides_permanently(b.rb);
        });
    PowerOfTwoDivider<size_t> p2d{ cfg_.nsubsteps };
    std::vector<CollisionGroup> result;
    result.reserve(2 + clusters.size());

    auto& non_colliders_group = result.emplace_back(CollisionGroup{
        .penetration_class = PenetrationClass::NONE,
        .nsubsteps = 1,
        .divider = cfg_.nsubsteps
    });
    for (auto& m : objects_) {
        auto it = collidable_modes_.find(&m.rigid_body.get());
        if (it == collidable_modes_.end()) {
            throw std::runtime_error("Could not determine collidable mode");
        }
        if (it->second == CollidableMode::MOVE) {
            non_colliders_group.rigid_bodies.insert(&m.rigid_body->rbp_);
        }
    }

    auto& bullet_line_group = result.emplace_back(CollisionGroup{
        .penetration_class = PenetrationClass::BULLET_LINE,
        .nsubsteps = 1,
        .divider = cfg_.nsubsteps,
        .rigid_bodies = std::move(bullet_line_segments)
    });

    // linfo() << "clusters";
    for (auto& c : clusters) {
        CollisionGroup g{
            .penetration_class = PenetrationClass::STANDARD,
            .nsubsteps = 0
        };
        // linfo() << "g";
        for (const auto& e : c) {
            if (bullet_line_group.rigid_bodies.contains(&e->rb.rbp_)) {
                throw std::runtime_error(
                    "Rigid body \"" + e->rb.name() + "\" contains both, bullet "
                    "line segments and standard meshes, which is not supported");
            }
            auto vmax = e->rb.rbp_.penetration_limits_.vmax_translation(cfg_.dt);
            auto wmax = e->rb.rbp_.penetration_limits_.wmax(cfg_.dt);
            // If radius == inf, which it is in avatars, wmax == 0.
            // In this case, the angular velocity is not taken into account.
            if (wmax == 0.f) {
                wmax = INFINITY;
            }
            auto nf = std::max(
                std::sqrt(sum(squared(e->rb.rbp_.v_com_))) / vmax,
                std::sqrt(sum(squared(e->rb.rbp_.w_))) / wmax);
            // linfo() << "  " << e->rb.name() << " - " << nf;
            if (nf - 1e-6 > integral_to_float<float>(cfg_.nsubsteps)) {
                throw std::runtime_error(
                    "Velocity or angular velocity of rigid body \"" + e->rb.name() +
                    "\" out of bounds. " +
                    "n (float): " + std::to_string(nf));
            }
            auto ni = p2d.greatest_divider(
                std::min(
                    cfg_.nsubsteps,
                    float_to_integral<size_t>(std::ceil((1.f + cfg_.max_velocity_increase) * nf))));
            e->rb.substep_history_.append(ni);
            g.nsubsteps = std::max(g.nsubsteps, max(e->rb.substep_history_));
            e->rb.get_rigid_pulses(g.rigid_bodies);
            g.meshes.insert(e->mesh.mesh.second.get());
        }
        g.divider = cfg_.nsubsteps / g.nsubsteps;
        if (g.divider * g.nsubsteps != cfg_.nsubsteps) {
            throw std::runtime_error("Error computing collision group substep divider");
        }
        // linfo() << "  n: " << g.nsubsteps;
        result.push_back(std::move(g));
    }
    return result;
}

void RigidBodies::notify_colliding_start() {
    if (is_colliding_) {
        throw std::runtime_error("Collision phase already started");
    }
    is_colliding_ = true;
}

void RigidBodies::notify_colliding_end() {
    if (!is_colliding_) {
        throw std::runtime_error("Collision phase already ended");
    }
    is_colliding_ = false;
}
