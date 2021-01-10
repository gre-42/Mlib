#include "Rigid_Bodies.hpp"
#include <Mlib/Geometry/Fixed_Cross.hpp>
#include <Mlib/Geometry/Homogeneous.hpp>
#include <Mlib/Geometry/Mesh/Colored_Vertex_Array.hpp>
#include <Mlib/Physics/Collision/Collidable_Mode.hpp>
#include <Mlib/Physics/Collision/Transformed_Mesh.hpp>
#include <Mlib/Physics/Misc/Rigid_Body.hpp>

using namespace Mlib;

RigidBodies::RigidBodies(const PhysicsEngineConfig& cfg)
: bvh_{{cfg.static_radius, cfg.static_radius, cfg.static_radius}, 10},
  cfg_{cfg}
{}

std::list<std::vector<CollisionTriangleSphere>> split_with_static_radius(
    const std::list<std::shared_ptr<ColoredVertexArray>>& cvas,
    const FixedArray<float, 4, 4>& tm,
    float static_radius)
{
    if (std::isnan(static_radius)) {
        throw std::runtime_error("Static objects require a non-NAN static_radius");
    }
    if (any(isnan(tm))) {
        throw std::runtime_error("Transformation matrix contains NAN values. Forgot to add rigid body to scene node?");
    }
    std::list<std::pair<FixedArray<float, 3>, std::list<CollisionTriangleSphere>>> centers;
    for (auto& m : cvas) {
        if (m->material.collide) {
            for (const auto& t : m->transformed_triangles_sphere(tm)) {
                bool sphere_found = false;
                for (auto& x : centers) {
                    if (sum(squared(t.bounding_sphere.center() - x.first)) < squared(static_radius)) {
                        x.second.push_back(t);
                        sphere_found = true;
                        break;
                    }
                }
                if (!sphere_found) {
                    centers.push_back(std::make_pair(t.bounding_sphere.center(), std::list{t}));
                }
            }
        }
    }
    std::list<std::vector<CollisionTriangleSphere>> result;
    for (const auto& x : centers) {
        std::vector<CollisionTriangleSphere> res{x.second.begin(), x.second.end()};
        result.push_back(std::move(res));
    }
    return result;
}

void RigidBodies::add_rigid_body(
    const std::shared_ptr<RigidBody>& rigid_body,
    const std::list<std::shared_ptr<ColoredVertexArray>>& hitbox,
    const std::list<std::shared_ptr<ColoredVertexArray>>& tirelines,
    CollidableMode collidable_mode)
{
    if (collidable_mode == CollidableMode::TERRAIN) {
        if (rigid_body->mass() != INFINITY) {
            throw std::runtime_error("Terrain requires infinite mass");
        }
        // if (!tirelines.empty()) {
        //     throw std::runtime_error("static rigid body has tirelines");
        // }
        if (cfg_.bvh) {
            for (auto& m : hitbox) {
                if (m->material.collide) {
                    for (const auto& t : m->transformed_triangles_bbox(rigid_body->get_new_absolute_model_matrix())) {
                        bvh_.insert(t.aabb, t.base);
                    }
                }
            }
            static_rigid_bodies_.push_back(rigid_body);
        } else {
            auto xx = split_with_static_radius(hitbox, rigid_body->get_new_absolute_model_matrix(), cfg_.static_radius);
            RigidBodyAndTransformedMeshes rbtm;
            rbtm.rigid_body = rigid_body;
            
            for (const auto& p : xx) {
                std::vector<FixedArray<float, 3>> vertices;
                vertices.reserve(p.size() * 3);
                for (const auto& t : p) {
                    vertices.push_back(t.triangle(0));
                    vertices.push_back(t.triangle(1));
                    vertices.push_back(t.triangle(2));
                }
                if (!vertices.empty()) {
                    BoundingSphere<float, 3> bs{vertices.begin(), vertices.end()};
                    rbtm.meshes.push_back({
                        .mesh_type = MeshType::CHASSIS,
                        .mesh = std::make_shared<TransformedMesh>(bs, p)});
                }
            }
            transformed_objects_.push_back(rbtm);
        }
    } else {
        RigidBodyAndMeshes rbm;
        rbm.rigid_body = rigid_body;
        auto ins = [this, &rbm](const auto& cvas, MeshType mesh_type) {
            for (auto& cva : cvas) {
                if (cva->material.collide) {
                    auto vertices = cva->vertices();
                    if (!vertices.empty()) {
                        BoundingSphere<float, 3> bs{vertices.begin(), vertices.end()};
                        rbm.meshes.push_back({
                            .mesh_type = mesh_type,
                            .mesh = std::make_pair(bs, cva)});
                    }
                }
            }
        };
        ins(hitbox, MeshType::CHASSIS);
        ins(tirelines, MeshType::TIRE_LINE);
        if (collidable_mode == CollidableMode::SMALL_STATIC) {
            if (rigid_body->mass() != INFINITY) {
                throw std::runtime_error("Small static requires infinite mass");
            }
            transform_object_and_add(rbm);
        } else if (collidable_mode == CollidableMode::SMALL_MOVING) {
            if (!std::isfinite(rigid_body->mass())) {
                throw std::runtime_error("Small moving requires finite mass");
            }
            objects_.push_back(std::move(rbm));
        }
    }
    if (!collidable_modes_.insert({rigid_body.get(), collidable_mode}).second) {
        throw std::runtime_error("Could not insert collidable mode");
    }
}

void RigidBodies::delete_rigid_body(const RigidBody* rigid_body) {
    auto it = collidable_modes_.find(rigid_body);
    if (it == collidable_modes_.end()) {
        throw std::runtime_error("Could not find rigid body for deletion");
    }
    if (rigid_body->mass() == INFINITY) {
        if (cfg_.bvh && it->second == CollidableMode::TERRAIN) {
            auto it = std::find_if(static_rigid_bodies_.begin(), static_rigid_bodies_.end(), [rigid_body](const auto& e){ return e.get() == rigid_body; });
            if (it == static_rigid_bodies_.end()) {
                throw std::runtime_error("Could not delete static rigid body (0)");
            }
            static_rigid_bodies_.erase(it);
        } else if (
            (!cfg_.bvh && (it->second == CollidableMode::TERRAIN)) ||
            (it->second == CollidableMode::SMALL_STATIC))
        {
            auto it = std::find_if(transformed_objects_.begin(), transformed_objects_.end(), [rigid_body](const auto& e){ return e.rigid_body.get() == rigid_body; });
            if (it == transformed_objects_.end()) {
                throw std::runtime_error("Could not delete static rigid body (1)");
            }
            transformed_objects_.erase(it);
        } else {
            throw std::runtime_error("Could not delete rigid body (3)");
        }
    } else if (it->second == CollidableMode::SMALL_MOVING) {
        auto it = std::find_if(objects_.begin(), objects_.end(), [rigid_body](const auto& e){ return e.rigid_body.get() == rigid_body; });
        if (it == objects_.end()) {
            throw std::runtime_error("Could not delete dynamic rigid body (4)");
        }
        objects_.erase(it);
    } else {
        throw std::runtime_error("Could not delete rigid body (5)");
    }
    collidable_modes_.erase(it);
}

void RigidBodies::transform_object_and_add(const RigidBodyAndMeshes& o) {
    FixedArray<float, 4, 4> m = o.rigid_body->get_new_absolute_model_matrix();
    std::list<TypedMesh<std::shared_ptr<TransformedMesh>>> transformed_meshes;
    for (const auto& msh : o.meshes) {
        transformed_meshes.push_back({
            .mesh_type = msh.mesh_type,
            .mesh = std::make_shared<TransformedMesh>(
                m,
                msh.mesh.first,
                msh.mesh.second)});
    }
    transformed_objects_.push_back({
        rigid_body: o.rigid_body,
        meshes: std::move(transformed_meshes)});
}

void RigidBodies::print_search_time() const {
    std::cout << "Search time: " << bvh_.search_time() << std::endl;
}
