#include "Physics_Engine.hpp"
#include <Mlib/Math/Orderable_Fixed_Array.hpp>
#include <Mlib/Physics/Handle_Line_Triangle_Intersection.hpp>
#include <Mlib/Physics/Interfaces/Advance_Time.hpp>
#include <Mlib/Physics/Interfaces/External_Force_Provider.hpp>
#include <Mlib/Physics/Misc/Rigid_Body.hpp>
#include <Mlib/Physics/Sat_Normals.hpp>
#include <Mlib/Physics/Transformed_Mesh.hpp>
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
    // Scene destruction happens after physics thread joining
    // => not all advance_times are deleted.
    // Objects are deleted, because their deletion is not scheduled.
    advance_times_.delete_scheduled_advance_times();
    if (check_objects_deleted_on_destruction_) {
        if (!rigid_bodies_.objects_.empty()) {
            std::cerr << "~PhysicsEngine: " << rigid_bodies_.objects_.size() << " objects still exist." << std::endl;
        }
        if (!advance_times_.advance_times_to_delete_.empty()) {
            std::cerr << "~PhysicsEngine: " << advance_times_.advance_times_to_delete_.size() << " advance_times_to_delete still exist." << std::endl;
        }
        if (!advance_times_.advance_times_.empty()) {
            std::cerr << "~PhysicsEngine: " << advance_times_.advance_times_.size() << " advance_times still exist." << std::endl;
        }
    }
}

static void handle_triangle_triangle_intersection(
    const std::shared_ptr<RigidBody>& o0,
    const std::shared_ptr<RigidBody>& o1,
    const CollisionTriangle& t0,
    const TypedMesh<std::shared_ptr<TransformedMesh>>& msh0,
    const TypedMesh<std::shared_ptr<TransformedMesh>>& msh1,
    std::vector<FixedArray<float, 3>>& beacons,
    const PhysicsEngineConfig& cfg,
    const SatTracker& st)
{
    for(const auto& t1 : msh1.mesh->get_triangles()) {
        if (!t1.bounding_sphere.intersects(t0.bounding_sphere)) {
            continue;
        }
        if (!t1.bounding_sphere.intersects(t0.plane)) {
            continue;
        }
        // Closed, triangulated surfaces contain every edge twice.
        // => Remove duplicates by checking the order.
        if (OrderableFixedArray{t1.triangle(1)} < OrderableFixedArray{t1.triangle(2)}) {
            HandleLineTriangleIntersection::handle({
                o0: o0,
                o1: o1,
                mesh0: msh0.mesh,
                mesh1: msh1.mesh,
                l1: FixedArray<FixedArray<float, 3>, 2>{t1.triangle(1), t1.triangle(2)},
                t0: t0.triangle,
                p0: t0.plane,
                cfg: cfg,
                st: st,
                beacons: beacons,
                tire_id: SIZE_MAX,
                mesh0_two_sided: t0.two_sided,
                lines_are_normals: false});
        }
        if (OrderableFixedArray{t1.triangle(2)} < OrderableFixedArray{t1.triangle(0)}) {
            HandleLineTriangleIntersection::handle({
                o0: o0,
                o1: o1,
                mesh0: msh0.mesh,
                mesh1: msh1.mesh,
                l1: FixedArray<FixedArray<float, 3>, 2>{t1.triangle(2), t1.triangle(0)},
                t0: t0.triangle,
                p0: t0.plane,
                cfg: cfg,
                st: st,
                beacons: beacons,
                tire_id: SIZE_MAX,
                mesh0_two_sided: t0.two_sided,
                lines_are_normals: false});
        }
        if (OrderableFixedArray{t1.triangle(0)} < OrderableFixedArray{t1.triangle(1)}) {
            HandleLineTriangleIntersection::handle({
                o0: o0,
                o1: o1,
                mesh0: msh0.mesh,
                mesh1: msh1.mesh,
                l1: FixedArray<FixedArray<float, 3>, 2>{t1.triangle(0), t1.triangle(1)},
                t0: t0.triangle,
                p0: t0.plane,
                cfg: cfg,
                st: st,
                beacons: beacons,
                tire_id: SIZE_MAX,
                mesh0_two_sided: t0.two_sided,
                lines_are_normals: false});
        }
    }
}

static void collide_objects(
    const RigidBodyAndTransformedMeshes& o0,
    const RigidBodyAndTransformedMeshes& o1,
    const PhysicsEngineConfig& cfg,
    const SatTracker& st,
    std::vector<FixedArray<float, 3>>& beacons)
{
    if (o0.rigid_body == o1.rigid_body) {
        return;
    }
    if ((o0.rigid_body->mass() == INFINITY) && (o1.rigid_body->mass() == INFINITY)) {
        return;
    }
    if (o1.rigid_body->mass() == INFINITY) {
        return;
    }
    for(const auto& msh1 : o1.meshes) {
        for(const auto& msh0 : o0.meshes) {
            if (msh0.mesh_type == MeshType::TIRE_LINE) {
                continue;
            }
            if (!msh0.mesh->intersects(*msh1.mesh)) {
                continue;
            }
            for(const auto& t0 : msh0.mesh->get_triangles()) {
                // Mesh-sphere <-> triangle-sphere intersection
                if (!msh1.mesh->intersects(t0.bounding_sphere)) {
                    continue;
                }
                // Mesh-sphere <-> triangle-plane intersection
                if (!msh1.mesh->intersects(t0.plane)) {
                    continue;
                }
                if (!cfg.collide_only_normals && (o0.rigid_body->mass() != INFINITY)) {
                    handle_triangle_triangle_intersection(
                        o0.rigid_body,
                        o1.rigid_body,
                        t0,
                        msh0,
                        msh1,
                        beacons,
                        cfg,
                        st);
                }
                const auto& lines = msh1.mesh->get_lines();
                if (msh1.mesh_type == MeshType::CHASSIS) {
                    for(const auto& l1 : lines) {
                        HandleLineTriangleIntersection::handle({
                            o0: o0.rigid_body,
                            o1: o1.rigid_body,
                            mesh0: msh0.mesh,
                            mesh1: msh1.mesh,
                            l1: l1,
                            t0: t0.triangle,
                            p0: t0.plane,
                            cfg: cfg,
                            st: st,
                            beacons: beacons,
                            tire_id: SIZE_MAX,
                            mesh0_two_sided: t0.two_sided,
                            lines_are_normals: true});
                    }
                } else if (msh1.mesh_type == MeshType::TIRE_LINE) {
                    size_t tire_id = 0;
                    for(const auto& l1 : lines) {
                        HandleLineTriangleIntersection::handle({
                            o0: o0.rigid_body,
                            o1: o1.rigid_body,
                            mesh0: msh0.mesh,
                            mesh1: msh1.mesh,
                            l1: l1,
                            t0: t0.triangle,
                            p0: t0.plane,
                            cfg: cfg,
                            st: st,
                            beacons: beacons,
                            tire_id: tire_id,
                            mesh0_two_sided: t0.two_sided,
                            lines_are_normals: true});
                        ++tire_id;
                    }
                } else {
                    throw std::runtime_error("Unknown mesh type");
                }
            }
        }
    }
}

void PhysicsEngine::collide(std::vector<FixedArray<float, 3>>& beacons, bool burn_in)
{
    for(auto it = rigid_bodies_.transformed_objects_.begin(); it != rigid_bodies_.transformed_objects_.end(); ) {
        if (it->rigid_body->mass() != INFINITY) {
            it = rigid_bodies_.transformed_objects_.erase(it);
        } else {
            ++it;
        }
    }
    {
        std::list<std::shared_ptr<RigidBody>> olist;
        for(const auto& o : rigid_bodies_.objects_) {
            assert_true(o.rigid_body->mass() != INFINITY);
            o.rigid_body->reset_forces();
            olist.push_back(o.rigid_body);
        }
        for(const auto& efp : external_force_providers_) {
            efp->increment_external_forces(olist, burn_in);
        }
    }
    for(const auto& o : rigid_bodies_.objects_) {
        assert_true(o.rigid_body->mass() != INFINITY);
        if (o.meshes.size() == 0) {
            std::cerr << "Skipping object without meshes" << std::endl;
        }
        FixedArray<float, 4, 4> m = o.rigid_body->get_new_absolute_model_matrix();
        std::list<TypedMesh<std::shared_ptr<TransformedMesh>>> transformed_meshes;
        for(const auto& msh : o.meshes) {
            transformed_meshes.push_back({
                mesh_type: msh.mesh_type,
                mesh: std::make_shared<TransformedMesh>(
                    m,
                    msh.mesh.first,
                    msh.mesh.second)});
        }
        rigid_bodies_.transformed_objects_.push_back({
            rigid_body: o.rigid_body,
            meshes: std::move(transformed_meshes)});
    }
    SatTracker st;
    if (cfg_.sat) {
        st.clear();
    }
    collide_forward_ = !collide_forward_;
    if (collide_forward_) {
        for(const auto& o0 : rigid_bodies_.transformed_objects_) {
            for(const auto& o1 : rigid_bodies_.transformed_objects_) {
                collide_objects(o0, o1, cfg_, st, beacons);
            }
        }
    } else {
        for(const auto& o0 : reverse(rigid_bodies_.transformed_objects_)) {
            for(const auto& o1 : reverse(rigid_bodies_.transformed_objects_)) {
                collide_objects(o0, o1, cfg_, st, beacons);
            }
        }
    }
}

void PhysicsEngine::move_rigid_bodies(std::vector<FixedArray<float, 3>>& beacons) {
    for(auto it = rigid_bodies_.objects_.begin(); it != rigid_bodies_.objects_.end(); ) {
        auto& o = *it++;
        if (o.rigid_body->mass() != INFINITY) {
            o.rigid_body->advance_time(
                cfg_.dt,
                cfg_.min_acceleration,
                cfg_.min_velocity,
                cfg_.min_angular_velocity,
                cfg_.sticky,
                cfg_.hand_break_velocity,
                beacons);
        }
    }
}

void PhysicsEngine::move_advance_times() {
    for(auto a : advance_times_.advance_times_) {
        a->advance_time(cfg_.dt);
    }
}

void PhysicsEngine::burn_in(float seconds) {
    for(const auto& o : rigid_bodies_.objects_) {
        for(auto& e : o.rigid_body->engines_) {
            e.second.set_surface_power(NAN);
        }
    }
    for(float time = 0; time < seconds; time += cfg_.dt) {
        {
            std::vector<FixedArray<float, 3>> beacons;
            collide(beacons, true);  // true = burn_in
        }
        if (time < seconds / 2) {
            for(const auto& o : rigid_bodies_.objects_) {
                o.rigid_body->rbi_.T_ = 0;
            }
        }
        {
            std::vector<FixedArray<float, 3>> beacons;
            move_rigid_bodies(beacons);
        }
    }
}

void PhysicsEngine::add_external_force_provider(ExternalForceProvider* efp)
{
    external_force_providers_.push_back(efp);
}
