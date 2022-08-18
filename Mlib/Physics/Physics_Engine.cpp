#include "Physics_Engine.hpp"
#include <Mlib/Geometry/Coordinates/Gl_Look_At.hpp>
#include <Mlib/Geometry/Coordinates/Rotate_Axis_Onto_Other_Axis.hpp>
#include <Mlib/Geometry/Intersection/Collision_Line.hpp>
#include <Mlib/Geometry/Physics_Material.hpp>
#include <Mlib/Increment_In_Destructor.hpp>
#include <Mlib/Math/Orderable_Fixed_Array.hpp>
#include <Mlib/Math/Quaternion.hpp>
#include <Mlib/Physics/Collision/Constraints.hpp>
#include <Mlib/Physics/Collision/Grind_Info.hpp>
#include <Mlib/Physics/Collision/Handle_Line_Triangle_Intersection.hpp>
#include <Mlib/Physics/Collision/Sat_Normals.hpp>
#include <Mlib/Physics/Collision/Transformed_Mesh.hpp>
#include <Mlib/Physics/Gravity.hpp>
#include <Mlib/Physics/Interfaces/Advance_Time.hpp>
#include <Mlib/Physics/Interfaces/Controllable.hpp>
#include <Mlib/Physics/Interfaces/External_Force_Provider.hpp>
#include <Mlib/Physics/Misc/Beacon.hpp>
#include <Mlib/Physics/Rigid_Body/Rigid_Body_Vehicle.hpp>
#include <Mlib/Physics/Units.hpp>
#include <Mlib/Reverse_Iterator.hpp>

using namespace Mlib;

PhysicsEngine::PhysicsEngine(
    const PhysicsEngineConfig& cfg,
    bool check_objects_deleted_on_destruction)
: rigid_bodies_{cfg},
  collision_query_{*this},
  cfg_{cfg},
  check_objects_deleted_on_destruction_{check_objects_deleted_on_destruction}
{}

PhysicsEngine::~PhysicsEngine() {
    // The physics thread calls "delete_scheduled_advance_times".
    // However, scene destruction (which schedules deletion) happens after physics thread joining
    // and before PhysicsEngine destruction.
    // => We need to call "delete_scheduled_advance_times" in the PhysicsEngine destructor (in the main thread).
    // No special handling is required for objects (i.e. rigid bodies), because their deletion is not scheduled,
    // but happens instantaneously.
    advance_times_.delete_scheduled_advance_times();
    if (check_objects_deleted_on_destruction_) {
        if (!rigid_bodies_.objects_.empty()) {
            std::cerr << "~PhysicsEngine: " << rigid_bodies_.objects_.size() << " objects still exist." << std::endl;
        }
        if (!advance_times_.advance_times_to_delete_.empty()) {
            std::cerr << "~PhysicsEngine: " << advance_times_.advance_times_to_delete_.size() << " advance_times_to_delete still exist." << std::endl;
        }
        if (!advance_times_.advance_times_shared_.empty()) {
            std::cerr << "~PhysicsEngine: " << advance_times_.advance_times_shared_.size() << " advance_times_shared still exist." << std::endl;
        }
        if (!advance_times_.advance_times_ptr_.empty()) {
            std::cerr << "~PhysicsEngine: " << advance_times_.advance_times_ptr_.size() << " advance_times_ptr still exist." << std::endl;
        }
    }
}

static void collide_triangle_and_triangles(
    RigidBodyVehicle& o0,
    RigidBodyVehicle& o1,
    const TypedMesh<std::shared_ptr<TransformedMesh>>& msh0,
    const TypedMesh<std::shared_ptr<TransformedMesh>>& msh1,
    const CollisionTriangleSphere& t0,
    const PhysicsEngineConfig& cfg,
    const SatTracker& st,
    std::list<Beacon>* beacons,
    std::list<std::unique_ptr<ContactInfo>>& contact_infos,
    std::unordered_map<const FixedArray<FixedArray<double, 3>, 2>*, IntersectionSceneAndContact>& raycast_intersections,
    std::unordered_map<RigidBodyVehicle*, GrindInfo>& grind_infos,
    BaseLog* base_log)
{
    if (!any(msh1.physics_material & PhysicsMaterial::OBJ_BULLET_MESH) &&
        (cfg.collide_only_normals || (o0.mass() == INFINITY)))
    {
        return;
    }
    // Mesh-sphere <-> triangle-sphere intersection
    if (!msh1.mesh->intersects(t0.bounding_sphere)) {
        return;
    }
    // Mesh-sphere <-> triangle-plane intersection
    if (!msh1.mesh->intersects(t0.plane)) {
        return;
    }
    for (const auto& t1 : msh1.mesh->get_triangles_sphere()) {
        if (!t1.bounding_sphere.intersects(t0.bounding_sphere)) {
            continue;
        }
        if (!t1.bounding_sphere.intersects(t0.plane)) {
            continue;
        }
        // Closed, triangulated surfaces contain every edge twice.
        // => Remove duplicates by checking the order.
        if (OrderableFixedArray{t1.triangle(1)} < OrderableFixedArray{t1.triangle(2)}) {
            handle_line_triangle_intersection({
                .o0 = o0,
                .o1 = o1,
                .mesh0 = msh0.mesh,
                .mesh1 = msh1.mesh,
                .l1 = FixedArray<FixedArray<double, 3>, 2>{t1.triangle(1), t1.triangle(2)},
                .t0 = t0.triangle,
                .p0 = t0.plane,
                .cfg = cfg,
                .st = st,
                .beacons = beacons,
                .contact_infos = contact_infos,
                .raycast_intersections = raycast_intersections,
                .grind_infos = grind_infos,
                .tire_id1 = SIZE_MAX,
                .mesh0_material = t0.physics_material,
                .mesh1_material = msh1.physics_material,
                .l1_is_normal = false,
                .default_collision_type = CollisionType::REFLECT,
                .base_log = base_log});
        }
        if (OrderableFixedArray{t1.triangle(2)} < OrderableFixedArray{t1.triangle(0)}) {
            handle_line_triangle_intersection({
                .o0 = o0,
                .o1 = o1,
                .mesh0 = msh0.mesh,
                .mesh1 = msh1.mesh,
                .l1 = FixedArray<FixedArray<double, 3>, 2>{t1.triangle(2), t1.triangle(0)},
                .t0 = t0.triangle,
                .p0 = t0.plane,
                .cfg = cfg,
                .st = st,
                .beacons = beacons,
                .contact_infos = contact_infos,
                .raycast_intersections = raycast_intersections,
                .grind_infos = grind_infos,
                .tire_id1 = SIZE_MAX,
                .mesh0_material = t0.physics_material,
                .mesh1_material = msh1.physics_material,
                .l1_is_normal = false,
                .default_collision_type = CollisionType::REFLECT,
                .base_log = base_log});
        }
        if (OrderableFixedArray{t1.triangle(0)} < OrderableFixedArray{t1.triangle(1)}) {
            handle_line_triangle_intersection({
                .o0 = o0,
                .o1 = o1,
                .mesh0 = msh0.mesh,
                .mesh1 = msh1.mesh,
                .l1 = FixedArray<FixedArray<double, 3>, 2>{t1.triangle(0), t1.triangle(1)},
                .t0 = t0.triangle,
                .p0 = t0.plane,
                .cfg = cfg,
                .st = st,
                .beacons = beacons,
                .contact_infos = contact_infos,
                .raycast_intersections = raycast_intersections,
                .grind_infos = grind_infos,
                .tire_id1 = SIZE_MAX,
                .mesh0_material = t0.physics_material,
                .mesh1_material = msh1.physics_material,
                .l1_is_normal = false,
                .default_collision_type = CollisionType::REFLECT,
                .base_log = base_log});
        }
    }
}

static void collide_triangle_and_lines(
    RigidBodyVehicle& o0,
    RigidBodyVehicle& o1,
    const TypedMesh<std::shared_ptr<TransformedMesh>>& msh0,
    const TypedMesh<std::shared_ptr<TransformedMesh>>& msh1,
    const CollisionTriangleSphere& t0,
    const PhysicsEngineConfig& cfg,
    const SatTracker& st,
    std::list<Beacon>* beacons,
    std::list<std::unique_ptr<ContactInfo>>& contact_infos,
    std::unordered_map<const FixedArray<FixedArray<double, 3>, 2>*, IntersectionSceneAndContact>& raycast_intersections,
    std::unordered_map<RigidBodyVehicle*, GrindInfo>& grind_infos,
    BaseLog* base_log)
{
    const auto& lines1 = msh1.mesh->get_lines_sphere();
    if (lines1.empty()) {
        return;
    }
    if (any(msh1.physics_material & PhysicsMaterial::OBJ_CHASSIS) ||
        any(msh1.physics_material & PhysicsMaterial::OBJ_BULLET_LINE_SEGMENT) ||
        any(msh1.physics_material & PhysicsMaterial::OBJ_ALIGNMENT_CONTACT) ||
        any(msh1.physics_material & PhysicsMaterial::OBJ_DISTANCEBOX))
    {
        for (const auto& l1 : lines1) {
            if (!l1.bounding_sphere.intersects(t0.bounding_sphere)) {
                continue;
            }
            if (!l1.bounding_sphere.intersects(t0.plane)) {
                continue;
            }
            handle_line_triangle_intersection({
                .o0 = o0,
                .o1 = o1,
                .mesh0 = msh0.mesh,
                .mesh1 = msh1.mesh,
                .l1 = l1.line,
                .t0 = t0.triangle,
                .p0 = t0.plane,
                .cfg = cfg,
                .st = st,
                .beacons = beacons,
                .contact_infos = contact_infos,
                .raycast_intersections = raycast_intersections,
                .grind_infos = grind_infos,
                .tire_id1 = SIZE_MAX,
                .mesh0_material = t0.physics_material,
                .mesh1_material = msh1.physics_material,
                .l1_is_normal = true,
                .default_collision_type = CollisionType::REFLECT,
                .base_log = base_log});
        }
    } else if (any(msh1.physics_material & PhysicsMaterial::OBJ_TIRE_LINE)) {
        size_t tire_id1 = 0;
        for (const auto& l1 : lines1) {
            IncrementInDestructor iid{tire_id1};
            if (!l1.bounding_sphere.intersects(t0.bounding_sphere)) {
                continue;
            }
            if (!l1.bounding_sphere.intersects(t0.plane)) {
                continue;
            }
            handle_line_triangle_intersection({
                .o0 = o0,
                .o1 = o1,
                .mesh0 = msh0.mesh,
                .mesh1 = msh1.mesh,
                .l1 = l1.line,
                .t0 = t0.triangle,
                .p0 = t0.plane,
                .cfg = cfg,
                .st = st,
                .beacons = beacons,
                .contact_infos = contact_infos,
                .raycast_intersections = raycast_intersections,
                .grind_infos = grind_infos,
                .tire_id1 = tire_id1,
                .mesh0_material = t0.physics_material,
                .mesh1_material = msh1.physics_material,
                .l1_is_normal = true,
                .default_collision_type = CollisionType::REFLECT,
                .base_log = base_log});
        }
        if (tire_id1 != o1.tires_.size()) {
            throw std::runtime_error(
                "Number of tire-lines (" + std::to_string(tire_id1) + ") does not equal the "
                "number of tires (" + std::to_string(o1.tires_.size()) + ") in object \"" + o1.name() + '"');
        }
    } else if (any(msh1.physics_material & PhysicsMaterial::OBJ_HITBOX)) {
        throw std::runtime_error("Detected hitbox with lines in object \"" + o1.name() + '"');
    } else {
        throw std::runtime_error(
            "Unknown mesh type when colliding objects \"" +
            o0.name() + "\" and \"" + o1.name() + '"');
    }
}

static void collide_line(
    RigidBodyVehicle& o0,
    RigidBodyVehicle& o1,
    const TypedMesh<std::shared_ptr<TransformedMesh>>& msh0,
    const TypedMesh<std::shared_ptr<TransformedMesh>>& msh1,
    const CollisionLineSphere& l0,
    const PhysicsEngineConfig& cfg,
    const SatTracker& st,
    std::list<Beacon>* beacons,
    std::list<std::unique_ptr<ContactInfo>>& contact_infos,
    std::unordered_map<const FixedArray<FixedArray<double, 3>, 2>*, IntersectionSceneAndContact>& raycast_intersections,
    std::unordered_map<RigidBodyVehicle*, GrindInfo>& grind_infos,
    BaseLog* base_log)
{
    // Mesh-sphere <-> line-sphere intersection
    if (!msh1.mesh->intersects(l0.bounding_sphere)) {
        return;
    }
    for (const auto& t1 : msh1.mesh->get_triangles_sphere()) {
        if (!t1.bounding_sphere.intersects(l0.bounding_sphere)) {
            continue;
        }
        handle_line_triangle_intersection({
            .o0 = o1,
            .o1 = o0,
            .mesh0 = msh1.mesh,
            .mesh1 = msh0.mesh,
            .l1 = l0.line,
            .t0 = t1.triangle,
            .p0 = t1.plane,
            .cfg = cfg,
            .st = st,
            .beacons = beacons,
            .contact_infos = contact_infos,
            .raycast_intersections = raycast_intersections,
            .grind_infos = grind_infos,
            .tire_id1 = SIZE_MAX,
            .mesh0_material = t1.physics_material,
            .mesh1_material = msh0.physics_material,
            .l1_is_normal = false,
            .default_collision_type = CollisionType::GRIND,
            .base_log = base_log});
    }
}

static void collide_objects(
    const RigidBodyAndTransformedMeshes& o0,
    const RigidBodyAndTransformedMeshes& o1,
    const PhysicsEngineConfig& cfg,
    const SatTracker& st,
    std::list<Beacon>* beacons,
    std::list<std::unique_ptr<ContactInfo>>& contact_infos,
    std::unordered_map<const FixedArray<FixedArray<double, 3>, 2>*, IntersectionSceneAndContact>& raycast_intersections,
    std::unordered_map<RigidBodyVehicle*, GrindInfo>& grind_infos,
    BaseLog* base_log)
{
    if (o0.rigid_body == o1.rigid_body) {
        return;
    }
    if ((o0.rigid_body->mass() == INFINITY) && (o1.rigid_body->mass() == INFINITY)) {
        return;
    }
    PhysicsMaterial included_materials =
        PhysicsMaterial::OBJ_BULLET_COLLIDABLE_MASK |
        PhysicsMaterial::OBJ_BULLET_MASK |
        PhysicsMaterial::OBJ_DISTANCEBOX;
    for (const auto& msh1 : o1.meshes) {
        if (!any(msh1.physics_material & included_materials)) {
            continue;
        }
        for (const auto& msh0 : o0.meshes) {
            if (!any(msh0.physics_material & included_materials)) {
                continue;
            }
            PhysicsMaterial combined_material = (msh0.physics_material | msh1.physics_material);
            if (any(combined_material & PhysicsMaterial::OBJ_BULLET_MASK) &&
               !any(combined_material & PhysicsMaterial::OBJ_BULLET_COLLIDABLE_MASK))
            {
                continue;
            }
            if (!msh0.mesh->intersects(*msh1.mesh)) {
                continue;
            }
            for (const auto& t0 : msh0.mesh->get_triangles_sphere()) {
                collide_triangle_and_triangles(
                    *o0.rigid_body,
                    *o1.rigid_body,
                    msh0,
                    msh1,
                    t0,
                    cfg,
                    st,
                    beacons,
                    contact_infos,
                    raycast_intersections,
                    grind_infos,
                    base_log);
                collide_triangle_and_lines(
                    *o0.rigid_body,
                    *o1.rigid_body,
                    msh0,
                    msh1,
                    t0,
                    cfg,
                    st,
                    beacons,
                    contact_infos,
                    raycast_intersections,
                    grind_infos,
                    base_log);
            }
            for (const auto& t1 : msh1.mesh->get_triangles_sphere()) {
                collide_triangle_and_lines(
                    *o1.rigid_body,
                    *o0.rigid_body,
                    msh1,
                    msh0,
                    t1,
                    cfg,
                    st,
                    beacons,
                    contact_infos,
                    raycast_intersections,
                    grind_infos,
                    base_log);
            }
        }
    }
}

void PhysicsEngine::collide(
    std::list<Beacon>* beacons,
    bool burn_in,
    size_t oversampling_iteration,
    BaseLog* base_log)
{
    rigid_bodies_.transformed_objects_.remove_if([](const RigidBodyAndTransformedMeshes& rbtm){
        return (rbtm.rigid_body->mass() != INFINITY);
    });
    {
        std::list<std::shared_ptr<RigidBodyVehicle>> olist;
        for (const auto& o : rigid_bodies_.objects_) {
            if (o.rigid_body->mass() != INFINITY) {
                o.rigid_body->reset_forces(oversampling_iteration);
                olist.push_back(o.rigid_body);
            }
        }
        for (const auto& co : controllables_) {
            co->notify_reset(burn_in, cfg_);
        }
        for (const auto& efp : external_force_providers_) {
            efp->increment_external_forces(olist, burn_in, cfg_);
        }
    }
    std::list<std::unique_ptr<ContactInfo>> contact_infos;
    for (const auto& o : rigid_bodies_.objects_) {
        if (o.rigid_body->mass() != INFINITY) {
            if (o.smeshes.empty() && o.dmeshes.empty()) {
                std::cerr << "WARNING: Object has no meshes" << std::endl;
            }
            rigid_bodies_.transform_object_and_add(o);
            o.rigid_body->collide_with_air(cfg_, contact_infos);
        }
    }
    std::unordered_map<const FixedArray<FixedArray<double, 3>, 2>*, IntersectionSceneAndContact> raycast_intersections;
    std::unordered_map<RigidBodyVehicle*, GrindInfo> grind_infos;
    SatTracker st;
    collide_with_movables(
        st,
        beacons,
        contact_infos,
        raycast_intersections,
        grind_infos,
        base_log);
    collide_with_terrain(
        st,
        beacons,
        contact_infos,
        raycast_intersections,
        grind_infos,
        base_log);
    // Handling rays before grind_infos so new grind_infos can be created
    // by rays also.
    collide_raycast_intersections(raycast_intersections);
    collide_grind_infos(contact_infos, grind_infos);
    solve_contacts(contact_infos, cfg_.dt / cfg_.oversampling);
}

void PhysicsEngine::collide_with_movables(
    const SatTracker& st,
    std::list<Beacon>* beacons,
    std::list<std::unique_ptr<ContactInfo>>& contact_infos,
    std::unordered_map<const FixedArray<FixedArray<double, 3>, 2>*, IntersectionSceneAndContact>& raycast_intersections,
    std::unordered_map<RigidBodyVehicle*, GrindInfo>& grind_infos,
    BaseLog* base_log)
{
    collide_forward_ = !collide_forward_;
    if (collide_forward_) {
        for (const auto& o0 : rigid_bodies_.transformed_objects_) {
            for (const auto& o1 : rigid_bodies_.transformed_objects_) {
                collide_objects(o0, o1, cfg_, st, beacons, contact_infos, raycast_intersections, grind_infos, base_log);
            }
        }
    } else {
        for (const auto& o0 : reverse(rigid_bodies_.transformed_objects_)) {
            for (const auto& o1 : reverse(rigid_bodies_.transformed_objects_)) {
                collide_objects(o0, o1, cfg_, st, beacons, contact_infos, raycast_intersections, grind_infos, base_log);
            }
        }
    }
}

void PhysicsEngine::collide_with_terrain(
    const SatTracker& st,
    std::list<Beacon>* beacons,
    std::list<std::unique_ptr<ContactInfo>>& contact_infos,
    std::unordered_map<const FixedArray<FixedArray<double, 3>, 2>*, IntersectionSceneAndContact>& raycast_intersections,
    std::unordered_map<RigidBodyVehicle*, GrindInfo>& grind_infos,
    BaseLog* base_log)
{
    static TypedMesh<std::shared_ptr<TransformedMesh>> o0_mesh;
    for (const auto& o1 : rigid_bodies_.transformed_objects_) {
        if (o1.rigid_body->mass() == INFINITY) {
            continue;
        }
        for (const auto& msh1 : o1.meshes) {
            PhysicsMaterial collide_with_terrain_triangle_mask =
                PhysicsMaterial::OBJ_CHASSIS |
                PhysicsMaterial::OBJ_TIRE_LINE |
                PhysicsMaterial::OBJ_BULLET_MASK |
                PhysicsMaterial::OBJ_ALIGNMENT_CONTACT |
                PhysicsMaterial::OBJ_DISTANCEBOX;
            if (any(msh1.physics_material & collide_with_terrain_triangle_mask))
            {
                auto bs1 = msh1.mesh->transformed_bounding_sphere();
                rigid_bodies_.triangle_bvh_.visit(
                    AxisAlignedBoundingBox{ bs1.center(), bs1.radius() },
                    [&](const RigidBodyAndCollisionTriangleSphere& t0){
                        collide_triangle_and_triangles(
                            t0.rb,
                            *o1.rigid_body,
                            o0_mesh,
                            msh1,
                            t0.ctp,
                            cfg_,
                            st,
                            beacons,
                            contact_infos,
                            raycast_intersections,
                            grind_infos,
                            base_log);
                        collide_triangle_and_lines(
                            t0.rb,
                            *o1.rigid_body,
                            o0_mesh,
                            msh1,
                            t0.ctp,
                            cfg_,
                            st,
                            beacons,
                            contact_infos,
                            raycast_intersections,
                            grind_infos,
                            base_log);
                        return true;
                    });
            } else if (any(msh1.physics_material & PhysicsMaterial::OBJ_GRIND_CONTACT)) {
                auto bs1 = msh1.mesh->transformed_bounding_sphere();
                rigid_bodies_.line_bvh_.visit(
                    AxisAlignedBoundingBox{ bs1.center(), bs1.radius() },
                    [&](const RigidBodyAndCollisionLineSphere& l0){
                        collide_line(
                            l0.rb,
                            *o1.rigid_body,
                            o0_mesh,
                            msh1,
                            l0.clp,
                            cfg_,
                            st,
                            beacons,
                            contact_infos,
                            raycast_intersections,
                            grind_infos,
                            base_log);
                        return true;
                    });
            } else if (any(msh1.physics_material & PhysicsMaterial::OBJ_HITBOX)) {
                if (!msh1.mesh->get_lines_sphere().empty()) {
                    throw std::runtime_error("Detected hitbox with lines in object \"" + o1.rigid_body->name() + '"');
                }
            } else {
                throw std::runtime_error(
                    "Unknown mesh type when colliding object \"" + o1.rigid_body->name() + '"');
            }
        }
    }
}

void PhysicsEngine::collide_raycast_intersections(
    const std::unordered_map<const FixedArray<FixedArray<double, 3>, 2>*, IntersectionSceneAndContact>& raycast_intersections)
{
    for (const auto& [l1, cc] : raycast_intersections) {
        handle_line_triangle_intersection(cc.scene, cc.intersection_point);
    }
}

void PhysicsEngine::collide_grind_infos(
    std::list<std::unique_ptr<ContactInfo>>& contact_infos,
    const std::unordered_map<RigidBodyVehicle*, GrindInfo>& grind_infos)
{
    for (const auto& [rb, p] : grind_infos) {
        rb->grind_state_.grind_pv_ = dot1d(rb->rbi_.rbp_.rotation_.T(), p.rail_direction.casted<float>());
        if (std::abs(rb->grind_state_.grind_pv_(0)) > std::abs(rb->grind_state_.grind_pv_(2))) {
            rb->grind_state_.grind_axis_ = 0;
        } else {
            rb->grind_state_.grind_axis_ = 2;
        }
        if (rb->jump_state_.wants_to_jump_oversampled_) {
            auto& o0 = *rb;
            auto& o1 = *p.rail_rb;
            auto point_dir = o0.rbi_.rbp_.rotation_.column(rb->grind_state_.grind_axis_);
            point_dir *= sign(dot0d(point_dir, o0.rbi_.rbp_.v_));
            point_dir -= dot0d(point_dir, p.rail_direction.casted<float>()) * p.rail_direction.casted<float>();
            auto n = -gravity_direction + point_dir * 2.f;
            n /= std::sqrt(sum(squared(n)));
            if (o1.mass() == INFINITY) {
                float mc = o0.rbi_.rbp_.effective_mass({ .vector = n, .position = p.intersection_point });
                float lambda = - std::max(0.f, mc * cfg_.grind_jump_dv);
                o0.rbi_.rbp_.integrate_impulse({
                    .vector = -n * lambda,
                    .position = p.intersection_point});
            } else {
                float mc0 = o0.rbi_.rbp_.effective_mass({ .vector = n, .position = p.intersection_point });
                float mc1 = o1.rbi_.rbp_.effective_mass({ .vector = n, .position = p.intersection_point });
                float lambda = - std::max(0.f, (mc0 * mc1 / (mc0 + mc1)) * cfg_.grind_jump_dv);
                o0.rbi_.rbp_.integrate_impulse({
                    .vector = -n * lambda,
                    .position = p.intersection_point});
                o1.rbi_.rbp_.integrate_impulse({
                    .vector = n * lambda,
                    .position = p.intersection_point});
            }
        } else if (rb->jump_state_.jumping_counter_ > 30 * cfg_.oversampling) {
            auto n = cross(p.rail_direction, FixedArray<double, 3>{ 0., 1., 0. });
            double l2 = sum(squared(n));
            if (l2 < 1e-12) {
                throw std::runtime_error("Rail normal too small");
            }
            n /= std::sqrt(l2);
            if (p.rail_rb->mass() == INFINITY) {
                if (!rb->align_to_surface_state_.touches_alignment_plane_) {
                    contact_infos.push_back(std::unique_ptr<ContactInfo>(new PlaneContactInfo1{
                        rb->rbi_.rbp_,
                        p.rail_rb->velocity_at_position(p.intersection_point),
                        BoundedPlaneEqualityConstraint{
                            .constraint =
                                PlaneEqualityConstraint{
                                .pec = PointEqualityConstraint{
                                    .p0 = rb->abs_grind_point(),
                                    .p1 = p.intersection_point,
                                    .beta = cfg_.plane_equality_beta},
                                .plane_normal = n.casted<float>()},
                            .lambda_min = rb->mass() * cfg_.velocity_lambda_min,
                            .lambda_max = -rb->mass() * cfg_.velocity_lambda_min
                        }}));
                    PlaneNd<double, 3> plane{
                        cross(n, p.rail_direction),
                        p.intersection_point};
                    contact_infos.push_back(std::unique_ptr<ContactInfo>(new NormalContactInfo1{
                        rb->rbi_.rbp_,
                        BoundedPlaneInequalityConstraint{
                            .constraint = PlaneInequalityConstraint{
                                .normal_impulse = NormalImpulse{.normal = plane.normal},
                                .intercept = plane.intercept,
                                .always_active = false},
                            .lambda_min = rb->mass() * cfg_.velocity_lambda_min,
                            .lambda_max = 0},
                        rb->abs_grind_point()}));
                } else {
                    contact_infos.push_back(std::unique_ptr<ContactInfo>(new LineContactInfo1{
                        rb->rbi_.rbp_,
                        p.rail_rb->velocity_at_position(p.intersection_point),
                        LineEqualityConstraint{
                            .pec = PointEqualityConstraint{
                                .p0 = rb->abs_grind_point(),
                                .p1 = p.intersection_point,
                                .beta = cfg_.point_equality_beta},
                            .line_direction = p.rail_direction.casted<float>()}}));
                }
            } else {
                if (!rb->align_to_surface_state_.touches_alignment_plane_) {
                    contact_infos.push_back(std::unique_ptr<ContactInfo>(new PlaneContactInfo2{
                        rb->rbi_.rbp_,
                        p.rail_rb->rbi_.rbp_,
                        BoundedPlaneEqualityConstraint{
                            .constraint =
                                PlaneEqualityConstraint{
                                    .pec = PointEqualityConstraint{
                                        .p0 = rb->abs_grind_point(),
                                        .p1 = p.intersection_point,
                                        .beta = cfg_.plane_equality_beta},
                                    .plane_normal = n.casted<float>()},
                            .lambda_min = (rb->mass() * p.rail_rb->mass()) / (rb->mass() + p.rail_rb->mass()) * cfg_.velocity_lambda_min,
                            .lambda_max = -(rb->mass() * p.rail_rb->mass()) / (rb->mass() + p.rail_rb->mass()) * cfg_.velocity_lambda_min
                        }}));
                    PlaneNd<double, 3> plane{
                        cross(n, p.rail_direction).casted<double>(),
                        p.intersection_point};
                    contact_infos.push_back(std::unique_ptr<ContactInfo>(new NormalContactInfo2{
                        rb->rbi_.rbp_,
                        p.rail_rb->rbi_.rbp_,
                        BoundedPlaneInequalityConstraint{
                            .constraint = PlaneInequalityConstraint{
                                .normal_impulse = NormalImpulse{.normal = plane.normal},
                                .intercept = plane.intercept,
                                .always_active = false},
                            .lambda_min = rb->mass() * cfg_.velocity_lambda_min,
                            .lambda_max = 0},
                        rb->abs_grind_point()}));
                } else {
                    contact_infos.push_back(std::unique_ptr<ContactInfo>(new LineContactInfo2{
                        rb->rbi_.rbp_,
                        p.rail_rb->rbi_.rbp_,
                        LineEqualityConstraint{
                            .pec = PointEqualityConstraint{
                                .p0 = rb->abs_grind_point(),
                                .p1 = p.intersection_point,
                                .beta = cfg_.point_equality_beta},
                            .line_direction = p.rail_direction.casted<float>()}}));
                }
            }
            rb->grind_state_.grinding_ = true;
            rb->grind_state_.grind_direction_ = p.rail_direction.casted<float>();
        }
    }
}

void PhysicsEngine::move_rigid_bodies(std::list<Beacon>* beacons) {
    for (const auto& rbm : rigid_bodies_.objects_) {
        auto& rb = rbm.rigid_body;
        assert_true(rb->mass() != INFINITY);
        // Revert surface power
        if ((rb->revert_surface_power_state_.revert_surface_power_threshold_ != INFINITY) &&
            (!rb->grind_state_.grinding_ || (rb->grind_state_.grind_axis_ == 2)))
        {
            float f = dot0d(rb->rbi_.rbp_.v_, dot1d(rb->rbi_.rbp_.rotation_, rb->tires_z_));
            if (!rb->revert_surface_power_state_.revert_surface_power_) {
                f = -f;
            }
            if (f > rb->revert_surface_power_state_.revert_surface_power_threshold_) {
                rb->revert_surface_power_state_.revert_surface_power_ = !rb->revert_surface_power_state_.revert_surface_power_;
            }
        }
        // Align to surface
        if (rb->grind_state_.grinding_) {
            if (rb->grind_state_.grind_axis_ == 0) {
                if (std::abs(rb->grind_state_.grind_pv_(0)) > 1e-12) {
                    auto x = cross(sign(rb->grind_state_.grind_pv_(0)) * rb->grind_state_.grind_direction_, gravity_direction);
                    x /= std::sqrt(sum(squared(x)));
                    auto z = cross(x, gravity_direction);
                    auto r1 = FixedArray<float, 3, 3>{
                        -z(0), -gravity_direction(0), -x(0),
                        -z(1), -gravity_direction(1), -x(1),
                        -z(2), -gravity_direction(2), -x(2)};
                    rb->rbi_.rbp_.rotation_ =
                        Quaternion<float>{ rb->rbi_.rbp_.rotation_ }
                        .slerp(Quaternion<float>{ r1 }, cfg_.alignment_slerp)
                        .to_rotation_matrix();
                }
            } else if (rb->grind_state_.grind_axis_ == 2) {
                if (std::abs(rb->grind_state_.grind_pv_(2)) > 1e-12) {
                    auto r1 = gl_lookat_relative(-sign(rb->grind_state_.grind_pv_(2)) * rb->grind_state_.grind_direction_, -gravity_direction);
                    rb->rbi_.rbp_.rotation_ =
                        Quaternion<float>{ rb->rbi_.rbp_.rotation_ }
                        .slerp(Quaternion<float>{ r1 }, cfg_.alignment_slerp)
                        .to_rotation_matrix();
                }
            } else {
                throw std::runtime_error("Unknown grind axis");
            }
        } else {
            if ((rb->align_to_surface_state_.align_to_surface_relaxation_ != 0) &&
                !all(Mlib::isnan(rb->align_to_surface_state_.surface_normal_)))
            {
                if (!all(rb->rbi_.rbp_.w_ == 0.f)) {
                    throw std::runtime_error("Detected angular velocity despite alignment to surface normal. Forgot to set the rigid body's size to INFINITY?");
                }
                rb->rbi_.rbp_.rotation_ = rotate_axis_onto_other_axis(
                    rb->rbi_.rbp_.rotation_,
                    rb->align_to_surface_state_.surface_normal_,
                    FixedArray<float, 3>{ 0.f, 1.f, 0.f },
                    rb->align_to_surface_state_.align_to_surface_relaxation_);
            }
        }
        // Advance time
        rb->advance_time(cfg_.dt / cfg_.oversampling, beacons);
    }
}

void PhysicsEngine::move_advance_times() {
    for (const auto& a : advance_times_.advance_times_shared_) {
        if (!advance_times_.advance_times_to_delete_.contains(a.get())) {
            a->advance_time(cfg_.dt);
        }
    }
    for (const auto& a : advance_times_.advance_times_ptr_) {
        if (!advance_times_.advance_times_to_delete_.contains(a)) {
            a->advance_time(cfg_.dt);
        }
    }
}

void PhysicsEngine::burn_in(float duration) {
    for (const auto& o : rigid_bodies_.objects_) {
        for (auto& [_, e] : o.rigid_body->engines_) {
            e.set_surface_power(NAN);
        }
    }
    for (float time = 0; time < duration; time += cfg_.dt / cfg_.oversampling) {
        collide(
            nullptr,        // beacons
            true,           // true = burn_in
            SIZE_MAX,       // oversampling_iteration
            nullptr);       // base_log
        if (time < duration / 2) {
            for (const auto& o : rigid_bodies_.objects_) {
                o.rigid_body->rbi_.T_ = 0;
                o.rigid_body->rbi_.rbp_.w_ = 0;
            }
        }
        move_rigid_bodies(nullptr);  // nullptr=beacons
    }
}

void PhysicsEngine::add_external_force_provider(ExternalForceProvider* efp)
{
    external_force_providers_.push_back(efp);
}

void PhysicsEngine::add_controllable(Controllable* co)
{
    if (!controllables_.insert(co).second) {
        throw std::runtime_error("Controllable already added");
    }
}

void PhysicsEngine::remove_controllable(Controllable* co) {
    if (controllables_.erase(co) != 1) {
        throw std::runtime_error("Controllable does not exist");
    }
}
