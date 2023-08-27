#include "Create_Scene_Rod.hpp"
#include <Mlib/Env.hpp>
#include <Mlib/Geometry/Cameras/Perspective_Camera.hpp>
#include <Mlib/Geometry/Colored_Vertex.hpp>
#include <Mlib/Geometry/Mesh/Colored_Vertex_Array.hpp>
#include <Mlib/Geometry/Mesh/Load/Load_Mesh_Config.hpp>
#include <Mlib/Geometry/Mesh/Load/Load_Obj.hpp>
#include <Mlib/Geometry/Physics_Material.hpp>
#include <Mlib/Physics/Collision/Collidable_Mode.hpp>
#include <Mlib/Physics/Physics_Engine/Physics_Engine.hpp>
#include <Mlib/Physics/Rigid_Body/Rigid_Body_Vehicle.hpp>
#include <Mlib/Physics/Rigid_Body/Rigid_Primitives.hpp>
#include <Mlib/Physics/Units.hpp>
#include <Mlib/Render/Resources/Colored_Vertex_Array_Resource.hpp>
#include <Mlib/Render/Resources/Obj_File_Resource.hpp>
#include <Mlib/Scene_Graph/Containers/Scene.hpp>
#include <Mlib/Scene_Graph/Elements/Absolute_Movable_Setter.hpp>
#include <Mlib/Scene_Graph/Elements/Light.hpp>
#include <Mlib/Scene_Graph/Elements/Scene_Node.hpp>
#include <Mlib/Scene_Graph/Instantiation_Options.hpp>
#include <Mlib/Scene_Graph/Interfaces/IScene_Node_Resource.hpp>
#include <Mlib/Scene_Graph/Resources/Renderable_Resource_Filter.hpp>
#include <Mlib/Scene_Graph/Resources/Scene_Node_Resources.hpp>

using namespace Mlib;

void Mlib::create_scene_rod(
    SceneNodeResources& scene_node_resources,
    Scene& scene,
    PhysicsEngine& pe,
    SelectedCameras& selected_cameras)
{
    auto rb0 = rigid_cuboid("ground", "ground_no_id", INFINITY, {1.f, 2.f, 3.f});
    auto rb1_0 = rigid_cuboid("rb0", "rb0_no_id", 3.f * kg, {2.f, 3.f, 4.f});

    auto load_box = [](
        const FixedArray<float, 3>& scale,
        PhysicsMaterial physics_material)
    {
        return load_obj(
            "Data/box.obj",
            LoadMeshConfig<float>{
                .scale = scale,
                .blend_mode = BlendMode::OFF,
                .cull_faces_default = true,
                .cull_faces_alpha = false,
                .occluded_pass = ExternalRenderPassType::LIGHTMAP_DEPTH,
                .occluder_pass = ExternalRenderPassType::LIGHTMAP_DEPTH,
                .aggregate_mode = AggregateMode::NONE,
                .transformation_mode = TransformationMode::ALL,
                .apply_static_lighting = true,
                .laplace_ao_strength = 0.f,
                .physics_material = physics_material,
                .werror = true});
    };
    std::list<std::shared_ptr<ColoredVertexArray<float>>> triangles1 = load_box(
        {1.f, 1.f, 1.f},
        PhysicsMaterial::ATTR_VISIBLE | PhysicsMaterial::ATTR_COLLIDE | PhysicsMaterial::OBJ_CHASSIS | PhysicsMaterial::ATTR_CONVEX);
    std::list<std::shared_ptr<ColoredVertexArray<float>>> triangles01 = load_box(
        {0.1f, 1.f, 0.1f},
        PhysicsMaterial::ATTR_VISIBLE | PhysicsMaterial::ATTR_COLLIDE | PhysicsMaterial::ATTR_CONCAVE);

    scene_node_resources.add_resource("obj0", std::make_shared<ColoredVertexArrayResource>(triangles01));
    scene_node_resources.add_resource("obj1", std::make_shared<ColoredVertexArrayResource>(triangles1));
    scene_node_resources.add_resource("beacon", load_renderable_obj(
        "Data/box.obj",
        LoadMeshConfig<float>{
            .position = FixedArray<float, 3>{0.f, 0.f, 0.f},
            .rotation = FixedArray<float, 3>{0.f, 0.f, 0.f},
            .scale = FixedArray<float, 3>{0.5f, 0.5f, 0.5f},
            .blend_mode = BlendMode::OFF,
            .cull_faces_default = true,
            .cull_faces_alpha = false,
            .occluded_pass = ExternalRenderPassType::NONE,
            .occluder_pass = ExternalRenderPassType::NONE,
            .aggregate_mode = AggregateMode::NONE,
            .transformation_mode = TransformationMode::ALL,
            .apply_static_lighting = true,
            .laplace_ao_strength = 0.f,
            .physics_material =  PhysicsMaterial::ATTR_VISIBLE | PhysicsMaterial::ATTR_COLLIDE,
            .werror = true},
        scene_node_resources));
    // scene_node_resources.generate_triangle_rays("obj1", 5, {1.f, 1.f, 1.f});
    auto scene_node0 = make_dunique<SceneNode>();
    auto scene_node1_0 = make_dunique<SceneNode>();
    auto scene_nodeR = make_dunique<SceneNode>();
    auto scene_nodeL = make_dunique<SceneNode>();

    scene_node_resources.instantiate_renderable("obj0", InstantiationOptions{
        .instance_name = "obj0",
        .scene_node = scene_node0.ref(DP_LOC),
        .renderable_resource_filter = RenderableResourceFilter{}});
    scene_node_resources.instantiate_renderable("obj1", InstantiationOptions{
        .instance_name = "obj1_0",
        .scene_node = scene_node1_0.ref(DP_LOC),
        .renderable_resource_filter = RenderableResourceFilter{}});
    scene_node0->set_rotation({0.f, 0.f, 0.001f * float(M_PI)});
    scene_node0->set_position({0., -4., 0.});
    scene_node1_0->set_rotation({0.f, 0.f, 0.1f * float(M_PI)});

    scene_nodeR->add_child("n0", std::move(scene_node0));
    scene_nodeR->add_child("n1_0", std::move(scene_node1_0));
    scene_nodeR->set_position({0.f, -1.f, -40.f});
    scene_nodeL->set_position({0.f, 50.f, -40.f});
    scene_nodeL->set_rotation({-90.f * degrees, 0.f, 0.f});
    scene_nodeL->add_light(std::make_unique<Light>(Light{
        .resource_suffix = "light_node",
        .shadow_render_pass = ExternalRenderPassType::LIGHTMAP_DEPTH}));
    scene_nodeL->add_light(std::make_unique<Light>(Light{
        .shadow_render_pass = ExternalRenderPassType::NONE}));

    scene.add_root_node("obj", std::move(scene_nodeR));
    scene.add_root_node("follower_camera", make_dunique<SceneNode>());
    scene.add_root_node("light_node", std::move(scene_nodeL));
    scene.get_node("follower_camera", DP_LOC)->set_camera(std::make_unique<PerspectiveCamera>(
        PerspectiveCameraConfig(),
        PerspectiveCamera::Postprocessing::ENABLED));
    scene.get_node("light_node", DP_LOC)->set_camera(std::make_unique<PerspectiveCamera>(
        PerspectiveCameraConfig(),
        PerspectiveCamera::Postprocessing::ENABLED));

    // Must be done when node is already linked to its parents.
    {
        AbsoluteMovableSetter ams0{scene.get_node("obj", DP_LOC)->get_child("n0"), std::move(rb0)};
        AbsoluteMovableSetter ams1_0{scene.get_node("obj", DP_LOC)->get_child("n1_0"), std::move(rb1_0)};

        pe.rigid_bodies_.add_rigid_body(std::move(ams0.absolute_movable), {triangles01}, {}, CollidableMode::STATIC);
        pe.rigid_bodies_.add_rigid_body(std::move(ams1_0.absolute_movable), triangles1, {}, CollidableMode::MOVING);
    }
}
