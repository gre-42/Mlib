#include "Create_Scene_Slide.hpp"
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

void Mlib::create_scene_slide(
    Scene& scene,
    PhysicsEngine& pe,
    SelectedCameras& selected_cameras)
{
    auto rb_slide = rigid_cuboid(global_object_pool, "slide", "slide_no_id", INFINITY, {1.f, 2.f, 3.f});
    auto rb_box = rigid_cuboid(global_object_pool, "box", "box_no_id", 3.f * kg, {2.f, 3.f, 4.f});

    auto load_slide = [](
        const FixedArray<float, 3>& scale,
        PhysicsMaterial physics_material)
        {
            return load_obj(
                getenv_default("SLIDE", "Data/slide.obj"),
                LoadMeshConfig<float>{
                .scale = scale,
                    .blend_mode = BlendMode::OFF,
                    .cull_faces_default = false,
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
    std::list<std::shared_ptr<ColoredVertexArray<float>>> triangles_slide = load_slide(
        {5.f, 1.f, 5.f},
        PhysicsMaterial::ATTR_VISIBLE | PhysicsMaterial::ATTR_COLLIDE | PhysicsMaterial::ATTR_CONCAVE);
    std::list<std::shared_ptr<ColoredVertexArray<float>>> triangles_box = load_box(
        {1.f, 1.f, 1.f},
        PhysicsMaterial::ATTR_VISIBLE | PhysicsMaterial::ATTR_COLLIDE | PhysicsMaterial::OBJ_CHASSIS | PhysicsMaterial::ATTR_CONVEX);

    RenderingContextStack::primary_scene_node_resources().add_resource("obj_slide", std::make_shared<ColoredVertexArrayResource>(triangles_slide));
    RenderingContextStack::primary_scene_node_resources().add_resource("obj_box", std::make_shared<ColoredVertexArrayResource>(triangles_box));
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
    auto scene_node_slide = make_unique_scene_node();
    auto scene_node_box = make_unique_scene_node();
    auto scene_node_light = make_unique_scene_node();

    RenderingContextStack::primary_scene_node_resources().instantiate_child_renderable("obj_slide", ChildInstantiationOptions{
        .rendering_resources = &RenderingContextStack::primary_rendering_resources(),
        .instance_name = VariableAndHash<std::string>{ "obj0" },
        .scene_node = scene_node_slide.ref(DP_LOC),
        .renderable_resource_filter = RenderableResourceFilter{}});
    RenderingContextStack::primary_scene_node_resources().instantiate_child_renderable("obj_box", ChildInstantiationOptions{
        .rendering_resources = &RenderingContextStack::primary_rendering_resources(),
        .instance_name = VariableAndHash<std::string>{ "obj1_0" },
        .scene_node = scene_node_box.ref(DP_LOC),
        .renderable_resource_filter = RenderableResourceFilter{}});
    scene_node_slide->set_rotation({0.f, -90.f * degrees, 0.f}, INITIAL_POSE);
    scene_node_slide->set_position({-5.f, 0.f, -90.f}, INITIAL_POSE);
    scene_node_box->set_rotation({0.f, 0.f, 10.f * degrees}, INITIAL_POSE);
    scene_node_box->set_position({-10.f, 10.f, -90.f}, INITIAL_POSE);

    scene_node_light->set_position({0.f, 50.f, -90.f}, INITIAL_POSE);
    scene_node_light->set_rotation({-90.f * degrees, 0.f, 0.f}, INITIAL_POSE);

    scene.auto_add_root_node("obj_slide", std::move(scene_node_slide), RenderingDynamics::STATIC);
    scene.auto_add_root_node("obj_box", std::move(scene_node_box), RenderingDynamics::MOVING);
    scene.add_root_node("follower_camera", make_unique_scene_node(), RenderingDynamics::MOVING, RenderingStrategies::OBJECT);
    scene.add_root_node("light_node", std::move(scene_node_light), RenderingDynamics::MOVING, RenderingStrategies::OBJECT);
    scene.get_node("follower_camera", DP_LOC)->set_camera(std::make_unique<PerspectiveCamera>(
        PerspectiveCameraConfig(),
        PerspectiveCamera::Postprocessing::ENABLED));
    scene.get_node("light_node", DP_LOC)->set_camera(std::make_unique<PerspectiveCamera>(
        PerspectiveCameraConfig(),
        PerspectiveCamera::Postprocessing::ENABLED));

    // Must be done when node is already linked to its parents.
    {
        AbsoluteMovableSetter ams_slide{scene.get_node("obj_slide", DP_LOC), std::move(rb_slide), CURRENT_SOURCE_LOCATION};
        AbsoluteMovableSetter ams_box{scene.get_node("obj_box", DP_LOC), std::move(rb_box), CURRENT_SOURCE_LOCATION};

        pe.rigid_bodies_.add_rigid_body(*ams_slide.absolute_movable, triangles_slide, {}, {}, CollidableMode::STATIC);
        pe.rigid_bodies_.add_rigid_body(*ams_box.absolute_movable, triangles_box, {}, {}, CollidableMode::MOVING);
        ams_slide.absolute_movable.release();
        ams_box.absolute_movable.release();
    }
}
