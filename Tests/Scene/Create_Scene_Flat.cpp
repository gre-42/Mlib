#include "Create_Scene_Flat.hpp"
#include <Mlib/Env.hpp>
#include <Mlib/Geometry/Cameras/Perspective_Camera.hpp>
#include <Mlib/Geometry/Colored_Vertex.hpp>
#include <Mlib/Geometry/Mesh/Colored_Vertex_Array.hpp>
#include <Mlib/Geometry/Mesh/Load/Load_Mesh_Config.hpp>
#include <Mlib/Geometry/Mesh/Load/Load_Obj.hpp>
#include <Mlib/Geometry/Physics_Material.hpp>
#include <Mlib/Geometry/Rectangle_Triangulation_Mode.hpp>
#include <Mlib/Math/Fixed_Test.hpp>
#include <Mlib/Physics/Collision/Collidable_Mode.hpp>
#include <Mlib/Physics/Physics_Engine/Physics_Engine.hpp>
#include <Mlib/Physics/Physics_Engine/Physics_Engine_Config.hpp>
#include <Mlib/Physics/Rigid_Body/Rigid_Body_Vehicle.hpp>
#include <Mlib/Physics/Rigid_Body/Rigid_Primitives.hpp>
#include <Mlib/Physics/Units.hpp>
#include <Mlib/Render/Rendering_Context.hpp>
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
#include <Mlib/Stats/Fast_Random_Number_Generators.hpp>

using namespace Mlib;

void Mlib::create_scene_flat(
    Scene& scene,
    PhysicsEngine& pe,
    SelectedCameras& selected_cameras,
    const PhysicsEngineConfig& physics_cfg,
    unsigned int seed)
{
    FastUniformRandomNumberGenerator<float> prng{seed, -1.f, 1.f};
    FastUniformRandomNumberGenerator<float> rrng{seed + 1, -0.1f * (float)M_PI, 0.1f * (float)M_PI};
    auto rb0 = rigid_cuboid("ground", "ground_no_id", INFINITY, {1.f, 2.f, 3.f});
    auto rb1_0 = rigid_cuboid("rb1_0", "rb1_0_no_id", 3.f * kg, {2.f, 3.f, 4.f});
    auto rb1_1 = rigid_cuboid("rb1_1", "rb1_1_no_id", 3.f * kg, {2.f, 3.f, 4.f});
    auto rb1_2 = rigid_cuboid("rb1_2", "rb1_2_no_id", 3.f * kg, {2.f, 3.f, 4.f});

    const auto z2 = fixed_zeros<float, 2>();
    const auto z3 = fixed_zeros<float, 3>();
    std::vector<FixedArray<ColoredVertex<float>, 3>> triangles0_raw{
        FixedArray<ColoredVertex<float>, 3>{
            ColoredVertex<float>{.position = {-10.f, -2.f, +10.f}, .color = {0.f, 0.f, 1.f}, .uv = z2, .normal = {0.f, 1.f, 0.f}, .tangent = z3},
            ColoredVertex<float>{.position = {+10.f, -2.f, -10.f}, .color = {0.f, 1.f, 0.f}, .uv = z2, .normal = {0.f, 1.f, 0.f}, .tangent = z3},
            ColoredVertex<float>{.position = {-10.f, -5.f, -10.f}, .color = {1.f, 0.f, 0.f}, .uv = z2, .normal = {0.f, 1.f, 0.f}, .tangent = z3}},
        FixedArray<ColoredVertex<float>, 3>{
            ColoredVertex<float>{.position = {+10.f, -2.f, -10.f}, .color = {0.f, 0.f, 1.f}, .uv = z2, .normal = {0.f, 1.f, 0.f}, .tangent = z3},
            ColoredVertex<float>{.position = {-10.f, -2.f, +10.f}, .color = {0.f, 1.f, 0.f}, .uv = z2, .normal = {0.f, 1.f, 0.f}, .tangent = z3},
            ColoredVertex<float>{.position = {+10.f, -5.f, +10.f}, .color = {1.f, 0.f, 0.f}, .uv = z2, .normal = {0.f, 1.f, 0.f}, .tangent = z3}}
    };
    auto triangles0 = std::make_shared<ColoredVertexArray<float>>(
        "triangles0",
        Material{
            .occluded_pass = ExternalRenderPassType::LIGHTMAP_DEPTH,
            .occluder_pass = ExternalRenderPassType::LIGHTMAP_DEPTH},
        PhysicsMaterial::ATTR_VISIBLE | PhysicsMaterial::ATTR_COLLIDE | PhysicsMaterial::OBJ_CHASSIS | PhysicsMaterial::ATTR_CONCAVE,
        ModifierBacklog{},
        std::vector<FixedArray<ColoredVertex<float>, 4>>(),
        std::move(triangles0_raw),
        std::vector<FixedArray<ColoredVertex<float>, 2>>(),
        std::vector<FixedArray<std::vector<BoneWeight>, 3>>(),
        std::vector<FixedArray<float, 3>>(),
        std::vector<FixedArray<uint8_t, 3>>());

    /*std::vector<FixedArray<ColoredVertex, 3>> triangles1{
        FixedArray<ColoredVertex, 3>{
            ColoredVertex{position: FixedArray<float, 3>{4, 0, 10}, color: FixedArray<float, 3>{0, 1, 1}},
            ColoredVertex{position: FixedArray<float, 3>{0, 4, 10}, color: FixedArray<float, 3>{0, 1, 1}},
            ColoredVertex{position: FixedArray<float, 3>{1, 1, -5}, color: FixedArray<float, 3>{1, 1, 0}}
        }
    };*/
    std::list<std::shared_ptr<ColoredVertexArray<float>>> triangles1 = load_obj(
        "Data/box.obj",
        LoadMeshConfig<float>{
            .blend_mode = BlendMode::OFF,
            .cull_faces_default = true,
            .cull_faces_alpha = false,
            .occluded_pass = ExternalRenderPassType::LIGHTMAP_DEPTH,
            .occluder_pass = ExternalRenderPassType::LIGHTMAP_DEPTH,
            .aggregate_mode = AggregateMode::NONE,
            .transformation_mode = TransformationMode::ALL,
            .apply_static_lighting = true,
            .laplace_ao_strength = 0.f,
            .physics_material = PhysicsMaterial::ATTR_VISIBLE | PhysicsMaterial::ATTR_COLLIDE | PhysicsMaterial::OBJ_CHASSIS | PhysicsMaterial::ATTR_CONVEX,
            .rectangle_triangulation_mode = RectangleTriangulationMode::DELAUNAY,
            .werror = true});
    std::list<std::shared_ptr<ColoredVertexArray<float>>> quads1 = load_obj(
        "Data/box.obj",
        LoadMeshConfig<float>{
            .blend_mode = BlendMode::OFF,
            .cull_faces_default = true,
            .cull_faces_alpha = false,
            .occluded_pass = ExternalRenderPassType::LIGHTMAP_DEPTH,
            .occluder_pass = ExternalRenderPassType::LIGHTMAP_DEPTH,
            .aggregate_mode = AggregateMode::NONE,
            .transformation_mode = TransformationMode::ALL,
            .apply_static_lighting = true,
            .laplace_ao_strength = 0.f,
            .physics_material = PhysicsMaterial::ATTR_COLLIDE | PhysicsMaterial::OBJ_CHASSIS | PhysicsMaterial::ATTR_CONVEX,
            .rectangle_triangulation_mode = RectangleTriangulationMode::DISABLED,
            .werror = true});

    RenderingContextStack::primary_scene_node_resources().add_resource("obj0", std::make_shared<ColoredVertexArrayResource>(triangles0));
    RenderingContextStack::primary_scene_node_resources().add_resource("obj1", std::make_shared<ColoredVertexArrayResource>(triangles1));
    RenderingContextStack::primary_scene_node_resources().add_resource("beacon", load_renderable_obj(
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
            .rectangle_triangulation_mode = RectangleTriangulationMode::DELAUNAY,
            .werror = true},
        RenderingContextStack::primary_scene_node_resources()));
    // RenderingContextStack::primary_scene_node_resources().generate_triangle_rays("obj1", 5, {1.f, 1.f, 1.f});
    auto scene_node0 = make_dunique<SceneNode>();
    auto scene_node1_0 = make_dunique<SceneNode>();
    auto scene_node1_1 = make_dunique<SceneNode>();
    auto scene_node1_2 = make_dunique<SceneNode>();
    auto scene_nodeR = make_dunique<SceneNode>();
    auto scene_nodeL = make_dunique<SceneNode>();

    RenderingContextStack::primary_scene_node_resources().instantiate_renderable("obj0", InstantiationOptions{
        .rendering_resources = &RenderingContextStack::primary_rendering_resources(),
        .instance_name = "obj0",
        .scene_node = scene_node0.ref(DP_LOC),
        .renderable_resource_filter = RenderableResourceFilter{}});
    RenderingContextStack::primary_scene_node_resources().instantiate_renderable("obj1", InstantiationOptions{
        .rendering_resources = &RenderingContextStack::primary_rendering_resources(),
        .instance_name = "obj1_0",
        .scene_node = scene_node1_0.ref(DP_LOC),
        .renderable_resource_filter = RenderableResourceFilter{}});
    RenderingContextStack::primary_scene_node_resources().instantiate_renderable("obj1", InstantiationOptions{
        .rendering_resources = &RenderingContextStack::primary_rendering_resources(),
        .instance_name = "obj1_1",
        .scene_node = scene_node1_1.ref(DP_LOC),
        .renderable_resource_filter = RenderableResourceFilter{}});
    RenderingContextStack::primary_scene_node_resources().instantiate_renderable("obj1", InstantiationOptions{
        .rendering_resources = &RenderingContextStack::primary_rendering_resources(),
        .instance_name = "obj1_2",
        .scene_node = scene_node1_2.ref(DP_LOC),
        .renderable_resource_filter = RenderableResourceFilter{}});
    if (getenv_default_bool("STACK", false)) {
        scene_node1_1->set_position(FixedArray<double, 3>{0., 4., 0.}, INITIAL_POSE);
        scene_node1_2->set_position(FixedArray<double, 3>{0., 8., 0.}, INITIAL_POSE);
    } else {
        scene_node0->set_rotation(FixedArray<float, 3>{0.f, 0.f, 0.001f * float(M_PI)}, INITIAL_POSE);
        scene_node1_1->set_position(FixedArray<double, 3>{0.1f + prng(), 4.f + prng(), 0.5f + prng()}, INITIAL_POSE);
        scene_node1_2->set_position(FixedArray<double, 3>{0.1f + prng(), 8.f + prng(), 0.5f + prng()}, INITIAL_POSE);
        scene_node1_0->set_rotation(FixedArray<float, 3>{0.f + rrng(), 0.f + rrng(), 0.1f * float(M_PI) + rrng()}, INITIAL_POSE);
        scene_node1_1->set_rotation(FixedArray<float, 3>{0.f + rrng(), 0.f + rrng(), 0.05f * float(M_PI) + rrng()}, INITIAL_POSE);
        scene_node1_2->set_rotation(FixedArray<float, 3>{0.f + rrng(), 0.f + rrng(), 0.05f * float(M_PI) + rrng()}, INITIAL_POSE);
    }

    scene_nodeR->add_child("n0", std::move(scene_node0));
    scene_nodeR->add_child("n1_0", std::move(scene_node1_0));
    scene_nodeR->add_child("n1_1", std::move(scene_node1_1));
    scene_nodeR->add_child("n1_2", std::move(scene_node1_2));
    scene_nodeR->set_position({0.f, -1.f, -40.f}, INITIAL_POSE);
    scene_nodeL->set_position({0.f, 50.f, -40.f}, INITIAL_POSE);
    scene_nodeL->set_rotation({-90.f * degrees, 0.f, 0.f}, INITIAL_POSE);
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
        AbsoluteMovableSetter ams1_1{scene.get_node("obj", DP_LOC)->get_child("n1_1"), std::move(rb1_1)};
        AbsoluteMovableSetter ams1_2{scene.get_node("obj", DP_LOC)->get_child("n1_2"), std::move(rb1_2)};

        pe.rigid_bodies_.add_rigid_body(std::move(ams0.absolute_movable), { triangles0 }, {}, CollidableMode::STATIC);
        pe.rigid_bodies_.add_rigid_body(std::move(ams1_0.absolute_movable), quads1, {}, CollidableMode::MOVING);
        pe.rigid_bodies_.add_rigid_body(std::move(ams1_1.absolute_movable), quads1, {}, CollidableMode::MOVING);
        pe.rigid_bodies_.add_rigid_body(std::move(ams1_2.absolute_movable), quads1, {}, CollidableMode::MOVING);
    }

    // Check if the initialization does not change the node positions.
    // Not that only "physics advance time" can change the positions.
    assert_allclose(scene.get_node("obj", DP_LOC)->get_child("n0")->position(), fixed_zeros<double, 3>());
    scene.move(physics_cfg.dt, std::chrono::steady_clock::now());
    assert_allclose(scene.get_node("obj", DP_LOC)->get_child("n0")->position(), fixed_zeros<double, 3>());
}
