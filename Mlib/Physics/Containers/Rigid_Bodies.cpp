#include "Rigid_Bodies.hpp"
#include <Mlib/Geometry/Coordinates/Homogeneous.hpp>
#include <Mlib/Geometry/Fixed_Cross.hpp>
#include <Mlib/Geometry/Intersection/Welzl.hpp>
#include <Mlib/Geometry/Mesh/Colored_Vertex_Array.hpp>
#include <Mlib/Geometry/Physics_Material.hpp>
#include <Mlib/Images/Svg.hpp>
#include <Mlib/Physics/Collision/Collidable_Mode.hpp>
#include <Mlib/Physics/Collision/Transformed_Mesh.hpp>
#include <Mlib/Physics/Rigid_Body/Rigid_Body_Vehicle.hpp>
#include <Mlib/Scene_Graph/Physics_Resource_Filter.hpp>

using namespace Mlib;

RigidBodies::RigidBodies(const PhysicsEngineConfig& cfg)
: triangle_bvh_{{cfg.bvh_max_size, cfg.bvh_max_size, cfg.bvh_max_size}, 7},
  line_bvh_{{cfg.bvh_max_size, cfg.bvh_max_size, cfg.bvh_max_size}, 7},
  cfg_{cfg}
{}

void RigidBodies::add_rigid_body(
    const std::shared_ptr<RigidBodyVehicle>& rigid_body,
    const std::list<std::shared_ptr<ColoredVertexArray<float>>>& s_hitboxes,
    const std::list<std::shared_ptr<ColoredVertexArray<double>>>& d_hitboxes,
    CollidableMode collidable_mode,
    const PhysicsResourceFilter& physics_resource_filter)
{
    if (collidable_mode == CollidableMode::TERRAIN) {
        if (rigid_body->mass() != INFINITY) {
            throw std::runtime_error("Terrain requires infinite mass");
        }
        // if (!tirelines.empty()) {
        //     throw std::runtime_error("static rigid body has tirelines");
        // }
        auto add_hitboxes = [&]<typename TPos>(const std::list<std::shared_ptr<ColoredVertexArray<TPos>>>& hitboxes) {
            for (auto& m : hitboxes) {
                if (physics_resource_filter.matches(*m)) {
                    if (any(m->physics_material & PhysicsMaterial::OBJ_GRIND_LINE)) {
                        for (const auto& t : m->transformed_lines_bbox(rigid_body->get_new_absolute_model_matrix())) {
                            line_bvh_.insert(t.aabb, {*rigid_body, t.base});
                        }
                    } else {
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
                        throw std::runtime_error("Alignment planes only supported for terrain");
                    }
                    auto vertices = cva->vertices();
                    if (!vertices.empty()) {
                        BoundingSphere<TPos, 3> bs = welzl_from_iterator<TPos, 3>(vertices.begin(), vertices.end());
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
        if (it->second == CollidableMode::TERRAIN) {
            auto it = std::find_if(static_rigid_bodies_.begin(), static_rigid_bodies_.end(), [rigid_body](const auto& e){ return e.get() == rigid_body; });
            if (it == static_rigid_bodies_.end()) {
                throw std::runtime_error("Could not delete static rigid body (0)");
            }
            static_rigid_bodies_.erase(it);
        } else if (it->second == CollidableMode::SMALL_STATIC)
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
    auto add_meshes = [&](const auto& meshes){
        for (const auto& msh : meshes) {
            transformed_meshes.push_back({
                .physics_material = msh.physics_material,
                .mesh = std::make_shared<TransformedMesh>(
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
    triangle_bvh_.optimize_search_time(ostr);
    line_bvh_.optimize_search_time(ostr);
}

void RigidBodies::print_search_time() const {
    std::cout << "Triangle search time: " << triangle_bvh_.search_time() << std::endl;
    std::cout << "Line search time: " << line_bvh_.search_time() << std::endl;
}

void RigidBodies::plot_triangle_bvh_svg(const std::string& filename, size_t axis0, size_t axis1) const {
    triangle_bvh_.plot_svg<double>(filename, axis0, axis1);
}

void RigidBodies::plot_line_bvh_svg(const std::string& filename, size_t axis0, size_t axis1) const {
    line_bvh_.plot_svg<double>(filename, axis0, axis1);
}
