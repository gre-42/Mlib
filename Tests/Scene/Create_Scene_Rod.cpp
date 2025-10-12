#include "Create_Scene_Rod.hpp"
#include <Mlib/Env.hpp>
#include <Mlib/Geometry/Cameras/Perspective_Camera.hpp>
#include <Mlib/Geometry/Colored_Vertex.hpp>
#include <Mlib/Geometry/Instance/Rendering_Dynamics.hpp>
#include <Mlib/Geometry/Mesh/Colored_Vertex_Array.hpp>
#include <Mlib/Geometry/Mesh/Load/Load_Mesh_Config.hpp>
#include <Mlib/Geometry/Mesh/Load/Load_Obj.hpp>
#include <Mlib/Geometry/Physics_Material.hpp>
#include <Mlib/Geometry/Rectangle_Triangulation_Mode.hpp>
#include <Mlib/Physics/Collision/Collidable_Mode.hpp>
#include <Mlib/Physics/Physics_Engine/Physics_Engine.hpp>
#include <Mlib/Physics/Rigid_Body/Rigid_Body_Vehicle.hpp>
#include <Mlib/Physics/Rigid_Body/Rigid_Primitives.hpp>
#include <Mlib/Physics/Units.hpp>
#include <Mlib/Render/Rendering_Context.hpp>
#include <Mlib/Render/Resources/Colored_Vertex_Array_Resource.hpp>
#include <Mlib/Render/Resources/Obj_File_Resource.hpp>
#include <Mlib/Scene_Graph/Containers/Scene.hpp>
#include <Mlib/Scene_Graph/Elements/Absolute_Movable_Setter.hpp>
#include <Mlib/Scene_Graph/Elements/Light.hpp>
#include <Mlib/Scene_Graph/Elements/Make_Scene_Node.hpp>
#include <Mlib/Scene_Graph/Elements/Rendering_Strategies.hpp>
#include <Mlib/Scene_Graph/Elements/Scene_Node.hpp>
#include <Mlib/Scene_Graph/Instantiation/Child_Instantiation_Options.hpp>
#include <Mlib/Scene_Graph/Interfaces/IScene_Node_Resource.hpp>
#include <Mlib/Scene_Graph/Resources/Renderable_Resource_Filter.hpp>
#include <Mlib/Scene_Graph/Resources/Scene_Node_Resources.hpp>

using namespace Mlib;

static const auto OBJ0 = VariableAndHash<std::string>{"obj0"};
static const auto OBJ1 = VariableAndHash<std::string>{"obj1"};
static const auto OBJ = VariableAndHash<std::string>{"obj"};
static const auto N0 = VariableAndHash<std::string>{"n0"};
static const auto N1_0 = VariableAndHash<std::string>{"n1_0"};
static const auto N1_1 = VariableAndHash<std::string>{"n1_1"};
static const auto BEACON = VariableAndHash<std::string>{"beacon"};

void Mlib::create_scene_rod(
    Scene& scene,
    PhysicsEngine& pe,
    SelectedCameras& selected_cameras)
{
    auto rb0 = rigid_cuboid(global_object_pool, "rb0", "ground_no_id", INFINITY, { 1.f, 2.f, 3.f });
    auto rb1_0 = rigid_cuboid(global_object_pool, "rb1_0", "rb1_0_no_id", 3.f * kg, { 2.f, 3.f, 4.f });

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
                .rectangle_triangulation_mode = RectangleTriangulationMode::DELAUNAY,
                .werror = true});
    };
    std::list<std::shared_ptr<ColoredVertexArray<float>>> triangles1 = load_box(
        {1.f, 1.f, 1.f},
        PhysicsMaterial::ATTR_VISIBLE | PhysicsMaterial::ATTR_COLLIDE | PhysicsMaterial::OBJ_CHASSIS | PhysicsMaterial::ATTR_CONVEX);
    std::list<std::shared_ptr<ColoredVertexArray<float>>> triangles01 = load_box(
        {0.1f, 1.f, 0.1f},
        PhysicsMaterial::ATTR_VISIBLE | PhysicsMaterial::ATTR_COLLIDE | PhysicsMaterial::ATTR_CONCAVE);

    RenderingContextStack::primary_scene_node_resources().add_resource(OBJ0, std::make_shared<ColoredVertexArrayResource>(triangles01));
    RenderingContextStack::primary_scene_node_resources().add_resource(OBJ1, std::make_shared<ColoredVertexArrayResource>(triangles1));
    RenderingContextStack::primary_scene_node_resources().add_resource(BEACON, load_renderable_obj(
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
    // scene_node_resources.generate_triangle_rays("obj1", 5, {1.f, 1.f, 1.f});
    auto scene_node0 = make_unique_scene_node();
    auto scene_node1_0 = make_unique_scene_node();
    auto scene_nodeR = make_unique_scene_node();
    auto scene_nodeL = make_unique_scene_node();

    RenderingContextStack::primary_scene_node_resources().instantiate_child_renderable(OBJ0, ChildInstantiationOptions{
        .rendering_resources = &RenderingContextStack::primary_rendering_resources(),
        .instance_name = OBJ0,
        .scene_node = scene_node0.ref(CURRENT_SOURCE_LOCATION),
        .renderable_resource_filter = RenderableResourceFilter{}});
    RenderingContextStack::primary_scene_node_resources().instantiate_child_renderable(OBJ1, ChildInstantiationOptions{
        .rendering_resources = &RenderingContextStack::primary_rendering_resources(),
        .instance_name = VariableAndHash<std::string>{ "obj1_0" },
        .scene_node = scene_node1_0.ref(CURRENT_SOURCE_LOCATION),
        .renderable_resource_filter = RenderableResourceFilter{}});
    scene_node0->set_rotation({0.f, 0.f, 0.001f * float(M_PI)}, INITIAL_POSE);
    scene_node0->set_position({0.f, -4.f, 0.f}, INITIAL_POSE);
    scene_node1_0->set_rotation({0.f, 0.f, 0.1f * float(M_PI)}, INITIAL_POSE);

    scene_nodeR->add_child(N0, std::move(scene_node0));
    scene_nodeR->add_child(N1_0, std::move(scene_node1_0));
    scene_nodeR->set_position({0.f, -1.f, -40.f}, INITIAL_POSE);
    scene_nodeL->set_position({0.f, 50.f, -40.f}, INITIAL_POSE);
    scene_nodeL->set_rotation({-90.f * degrees, 0.f, 0.f}, INITIAL_POSE);

    scene.auto_add_root_node(OBJ, std::move(scene_nodeR), RenderingDynamics::MOVING);
    scene.add_root_node(VariableAndHash<std::string>{"follower_camera_0"}, make_unique_scene_node(), RenderingDynamics::MOVING, RenderingStrategies::OBJECT);
    scene.add_root_node(VariableAndHash<std::string>{"light_node"}, std::move(scene_nodeL), RenderingDynamics::MOVING, RenderingStrategies::OBJECT);
    scene.get_node(VariableAndHash<std::string>{"follower_camera_0"}, DP_LOC)->set_camera(std::make_unique<PerspectiveCamera>(
        PerspectiveCameraConfig(),
        PerspectiveCamera::Postprocessing::ENABLED));
    scene.get_node(VariableAndHash<std::string>{"light_node"}, DP_LOC)->set_camera(std::make_unique<PerspectiveCamera>(
        PerspectiveCameraConfig(),
        PerspectiveCamera::Postprocessing::ENABLED));

    // Must be done when node is already linked to its parents.
    {
        AbsoluteMovableSetter ams0{scene, scene.get_node(OBJ, DP_LOC)->get_child(N0), N0, std::move(rb0), CURRENT_SOURCE_LOCATION};
        AbsoluteMovableSetter ams1_0{scene, scene.get_node(OBJ, DP_LOC)->get_child(N1_0), N1_0, std::move(rb1_0), CURRENT_SOURCE_LOCATION};

        pe.rigid_bodies_.add_rigid_body(*ams0.absolute_movable, triangles01, {}, {}, CollidableMode::COLLIDE);
        pe.rigid_bodies_.add_rigid_body(*ams1_0.absolute_movable, triangles1, {}, {}, CollidableMode::COLLIDE | CollidableMode::MOVE);
        ams0.absolute_movable.release();
        ams1_0.absolute_movable.release();
    }
}
