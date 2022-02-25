#include "Rigid_Bodies.hpp"
#include <Mlib/Geometry/Coordinates/Homogeneous.hpp>
#include <Mlib/Geometry/Fixed_Cross.hpp>
#include <Mlib/Geometry/Mesh/Colored_Vertex_Array.hpp>
#include <Mlib/Geometry/Physics_Material.hpp>
#include <Mlib/Images/Svg.hpp>
#include <Mlib/Physics/Collision/Collidable_Mode.hpp>
#include <Mlib/Physics/Collision/Transformed_Mesh.hpp>
#include <Mlib/Physics/Containers/Rigid_Body_Resource_Filter.hpp>
#include <Mlib/Physics/Rigid_Body/Rigid_Body_Vehicle.hpp>

using namespace Mlib;

RigidBodies::RigidBodies(const PhysicsEngineConfig& cfg)
: triangle_bvh_{{cfg.bvh_max_size, cfg.bvh_max_size, cfg.bvh_max_size}, 7},
  line_bvh_{{cfg.bvh_max_size, cfg.bvh_max_size, cfg.bvh_max_size}, 7},
  cfg_{cfg}
{}

static std::list<std::vector<CollisionTriangleSphere>> split_with_static_radius(
    const std::list<std::shared_ptr<ColoredVertexArray>>& cvas,
    const TransformationMatrix<float, 3>& tm,
    float static_radius,
    const RigidBodyResourceFilter& rigid_body_resource_filter)
{
    if (std::isnan(static_radius)) {
        throw std::runtime_error("Static objects require a non-NAN static_radius");
    }
    if (any(Mlib::isnan(tm.R())) || any(Mlib::isnan(tm.t()))) {
        throw std::runtime_error("Transformation matrix contains NAN values. Forgot to add rigid body to scene node?");
    }
    std::list<std::pair<FixedArray<float, 3>, std::list<CollisionTriangleSphere>>> centers;
    for (auto& m : cvas) {
        if (rigid_body_resource_filter.matches(*m)) {
            PhysicsMaterial pm = PhysicsMaterial::SOLID;
            if (!m->material.cull_faces) {
                pm |= PhysicsMaterial::TWO_SIDED;
            }
            for (const auto& t : m->transformed_triangles_sphere(tm, pm)) {
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
    const std::shared_ptr<RigidBodyVehicle>& rigid_body,
    const std::list<std::shared_ptr<ColoredVertexArray>>& hitbox,
    const std::list<std::shared_ptr<ColoredVertexArray>>& tirelines,
    const std::list<std::shared_ptr<ColoredVertexArray>>& grind_contacts,
    const std::list<std::shared_ptr<ColoredVertexArray>>& grind_lines,
    const std::list<std::shared_ptr<ColoredVertexArray>>& alignment_contacts,
    const std::list<std::shared_ptr<ColoredVertexArray>>& alignment_planes,
    CollidableMode collidable_mode,
    const RigidBodyResourceFilter& rigid_body_resource_filter)
{
    if (collidable_mode == CollidableMode::TERRAIN) {
        if (rigid_body->mass() != INFINITY) {
            throw std::runtime_error("Terrain requires infinite mass");
        }
        // if (!tirelines.empty()) {
        //     throw std::runtime_error("static rigid body has tirelines");
        // }
        if (cfg_.bvh) {
            auto ins = [this, &rigid_body, &rigid_body_resource_filter](
                const auto& cvas,
                PhysicsMaterial pm0)
            {
                for (auto& m : cvas) {
                    if (rigid_body_resource_filter.matches(*m)) {
                        PhysicsMaterial pm = pm0;
                        if (!m->material.cull_faces) {
                            pm |= PhysicsMaterial::TWO_SIDED;
                        }
                        for (const auto& t : m->transformed_triangles_bbox(rigid_body->get_new_absolute_model_matrix(), pm)) {
                            triangle_bvh_.insert(t.aabb, {*rigid_body, t.base});
                        }
                    }
                }
            };
            ins(hitbox, PhysicsMaterial::SOLID);
            ins(alignment_planes, PhysicsMaterial::ALIGNMENT_PLANE);
            for (auto& m : grind_lines) {
                if (rigid_body_resource_filter.matches(*m)) {
                    for (const auto& t : m->transformed_lines_bbox(rigid_body->get_new_absolute_model_matrix())) {
                        line_bvh_.insert(t.aabb, {*rigid_body, t.base});
                    }
                }
            }
            static_rigid_bodies_.push_back(rigid_body);
        } else {
            auto xx = split_with_static_radius(
                hitbox,
                rigid_body->get_new_absolute_model_matrix(),
                10 * cfg_.static_radius,
                rigid_body_resource_filter);
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
        auto ins = [this, &rbm, &rigid_body_resource_filter](const auto& cvas, MeshType mesh_type) {
            for (auto& cva : cvas) {
                if (rigid_body_resource_filter.matches(*cva)) {
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
        ins(grind_contacts, MeshType::GRIND_CONTACT);
        ins(grind_lines, MeshType::GRIND_LINE);
        ins(alignment_contacts, MeshType::ALIGNMENT_CONTACT);
        if (!alignment_planes.empty()) {
            throw std::runtime_error("Alignment planes only supported for terrain");
        }
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
    rigid_body->set_rigid_bodies(*this);
}

void RigidBodies::delete_rigid_body(const RigidBodyVehicle* rigid_body) {
    auto it = collidable_modes_.find(rigid_body);
    if (it == collidable_modes_.end()) {
        throw std::runtime_error("Could not find rigid body for deletion");
    }
    if (rigid_body->mass() == INFINITY) {
        if (cfg_.bvh && (it->second == CollidableMode::TERRAIN)) {
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
        {
            auto it = std::find_if(objects_.begin(), objects_.end(), [rigid_body](const auto& e){ return e.rigid_body.get() == rigid_body; });
            if (it == objects_.end()) {
                throw std::runtime_error("Could not delete dynamic rigid body (4)");
            }
            objects_.erase(it);
        }
        transformed_objects_.remove_if([rigid_body](const RigidBodyAndTransformedMeshes& rbtm){
            return (rbtm.rigid_body.get() == rigid_body);
        });
    } else {
        throw std::runtime_error("Could not delete rigid body (5)");
    }
    collidable_modes_.erase(it);
}

void RigidBodies::transform_object_and_add(const RigidBodyAndMeshes& o) {
    auto m = o.rigid_body->get_new_absolute_model_matrix();
    std::list<TypedMesh<std::shared_ptr<TransformedMesh>>> transformed_meshes;
    for (const auto& msh : o.meshes) {
        PhysicsMaterial pm = PhysicsMaterial::SOLID;
        if (!msh.mesh.second->material.cull_faces) {
            pm |= PhysicsMaterial::TWO_SIDED;
        }
        transformed_meshes.push_back({
            .mesh_type = msh.mesh_type,
            .mesh = std::make_shared<TransformedMesh>(
                m,
                msh.mesh.first,
                msh.mesh.second,
                pm)});
    }
    transformed_objects_.push_back({
        .rigid_body = o.rigid_body,
        .meshes = std::move(transformed_meshes)});
}

void RigidBodies::optimize_search_time(std::ostream& ostr) const {
    triangle_bvh_.optimize_search_time(ostr);
    line_bvh_.optimize_search_time(ostr);
}

void RigidBodies::print_search_time() const {
    std::cout << "Triangle search time: " << triangle_bvh_.search_time() << std::endl;
    std::cout << "Line search time: " << line_bvh_.search_time() << std::endl;
}

void RigidBodies::plot_triangle_bvh_svg(const std::string& filename, size_t axis0, size_t axis1) const {
    triangle_bvh_.plot_svg<float>(filename, axis0, axis1);
}

void RigidBodies::plot_line_bvh_svg(const std::string& filename, size_t axis0, size_t axis1) const {
    line_bvh_.plot_svg<float>(filename, axis0, axis1);
}
