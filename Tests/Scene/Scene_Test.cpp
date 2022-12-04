#include <Mlib/Env.hpp>
#include <Mlib/Floating_Point_Exceptions.hpp>
#include <Mlib/Fps/Set_Fps.hpp>
#include <Mlib/Geometry/Cameras/Perspective_Camera.hpp>
#include <Mlib/Geometry/Colored_Vertex.hpp>
#include <Mlib/Geometry/Mesh/Load_Mesh_Config.hpp>
#include <Mlib/Geometry/Mesh/Load_Obj.hpp>
#include <Mlib/Geometry/Physics_Material.hpp>
#include <Mlib/Images/Draw_Bmp.hpp>
#include <Mlib/Math/Fixed_Test.hpp>
#include <Mlib/Math/Pi.hpp>
#include <Mlib/Physics/Collision/Collidable_Mode.hpp>
#include <Mlib/Physics/Collision/Power_To_Force.hpp>
#include <Mlib/Physics/Gravity.hpp>
#include <Mlib/Physics/Misc/Gravity_Efp.hpp>
#include <Mlib/Physics/Physics_Engine/Physics_Engine.hpp>
#include <Mlib/Physics/Physics_Engine/Physics_Iteration.hpp>
#include <Mlib/Physics/Physics_Engine/Physics_Loop.hpp>
#include <Mlib/Physics/Rigid_Body/Rigid_Body_Vehicle.hpp>
#include <Mlib/Physics/Rigid_Body/Rigid_Primitives.hpp>
#include <Mlib/Render/Render2.hpp>
#include <Mlib/Render/Render_Config.hpp>
#include <Mlib/Render/Render_Logics/Clear_Mode.hpp>
#include <Mlib/Render/Render_Logics/Flying_Camera_Logic.hpp>
#include <Mlib/Render/Render_Logics/Lightmap_Logic.hpp>
#include <Mlib/Render/Render_Logics/Read_Pixels_Logic.hpp>
#include <Mlib/Render/Render_Logics/Render_Logics.hpp>
#include <Mlib/Render/Render_Logics/Standard_Camera_Logic.hpp>
#include <Mlib/Render/Render_Logics/Standard_Render_Logic.hpp>
#include <Mlib/Render/Render_Results.hpp>
#include <Mlib/Render/Rendering_Context.hpp>
#include <Mlib/Render/Rendering_Resources.hpp>
#include <Mlib/Render/Resources/Colored_Vertex_Array_Resource.hpp>
#include <Mlib/Render/Resources/Obj_File_Resource.hpp>
#include <Mlib/Render/Selected_Cameras.hpp>
#include <Mlib/Render/Ui/Button_States.hpp>
#include <Mlib/Render/Ui/Cursor_States.hpp>
#include <Mlib/Scene_Graph/Containers/Scene.hpp>
#include <Mlib/Scene_Graph/Delete_Node_Mutex.hpp>
#include <Mlib/Scene_Graph/Elements/Light.hpp>
#include <Mlib/Scene_Graph/Elements/Scene_Node.hpp>
#include <Mlib/Scene_Graph/Focus.hpp>
#include <Mlib/Scene_Graph/Instantiation_Options.hpp>
#include <Mlib/Scene_Graph/Physics_Resource_Filter.hpp>
#include <Mlib/Scene_Graph/Renderable_Resource_Filter.hpp>
#include <Mlib/Scene_Graph/Scene_Node_Resources.hpp>
#include <Mlib/Threads/Termination_Manager.hpp>
#include <atomic>
#include <thread>

using namespace Mlib;

void test_physics_engine() {
    std::atomic_size_t num_renderings = SIZE_MAX;
    RenderResults render_results;
    RenderedSceneDescriptor rsd;
    bool is_interactive = getenv_default_bool("PHYSICS_INTERACTIVE", false);
    if (!is_interactive) {
        render_results.outputs[rsd] = {};
    }
    RenderConfig render_config{
        .nsamples_msaa = 1,
        .cull_faces = BoolRenderOption::UNCHANGED,
        .wire_frame = BoolRenderOption::UNCHANGED,
        .window_title = "Physics test",
        .show_mouse_cursor = true};
    Render2 render2{
        render_config,
        num_renderings,
        &render_results};

    PhysicsEngineConfig physics_cfg{
        .dt = getenv_default_float("DT", 0.01667) * s,
        .oversampling = getenv_default_size_t("OVERSAMPLING", 20)};
    // SceneNode destructors require that physics engine is destroyed after scene,
    // => Create PhysicsEngine before Scene
    PhysicsEngine pe{physics_cfg};

    std::shared_ptr<RigidBodyVehicle> rb0 = rigid_cuboid("ground", INFINITY, {1, 2, 3});
    std::shared_ptr<RigidBodyVehicle> rb1_0 = rigid_cuboid("rb0", 3.f * kg, {2, 3, 4});
    std::shared_ptr<RigidBodyVehicle> rb1_1 = rigid_cuboid("rb1", 3.f * kg, {2, 3, 4});
    std::shared_ptr<RigidBodyVehicle> rb1_2 = rigid_cuboid("rb2", 3.f * kg, {2, 3, 4});

    std::vector<FixedArray<ColoredVertex<float>, 3>> triangles0_raw{
        FixedArray<ColoredVertex<float>, 3>{
            ColoredVertex<float>{.position = {-10, -2, +10}, .color = {0, 0, 1}, .normal = {0, 1, 0}},
            ColoredVertex<float>{.position = {+10, -2, -10}, .color = {0, 1, 0}, .normal = {0, 1, 0}},
            ColoredVertex<float>{.position = {-10, -2, -10}, .color = {1, 0, 0}, .normal = {0, 1, 0}}},
        FixedArray<ColoredVertex<float>, 3>{
            ColoredVertex<float>{.position = {+10, -2, -10}, .color = {0, 0, 1}, .normal = {0, 1, 0}},
            ColoredVertex<float>{.position = {-10, -2, +10}, .color = {0, 1, 0}, .normal = {0, 1, 0}},
            ColoredVertex<float>{.position = {+10, -2, +10}, .color = {1, 0, 0}, .normal = {0, 1, 0}}}
    };
    auto triangles0 = std::make_shared<ColoredVertexArray<float>>(
        "triangles0",
        Material{
            .occluded_pass = ExternalRenderPassType::LIGHTMAP_DEPTH,
            .occluder_pass = ExternalRenderPassType::LIGHTMAP_DEPTH},
        PhysicsMaterial::ATTR_VISIBLE | PhysicsMaterial::ATTR_COLLIDE | PhysicsMaterial::OBJ_CHASSIS | PhysicsMaterial::ATTR_CONVEX,
        std::move(triangles0_raw),
        std::move(std::vector<FixedArray<ColoredVertex<float>, 2>>()),
        std::move(std::vector<FixedArray<std::vector<BoneWeight>, 3>>()),
        std::move(std::vector<FixedArray<std::vector<BoneWeight>, 2>>()));

    /*std::vector<FixedArray<ColoredVertex, 3>> triangles1{
        FixedArray<ColoredVertex, 3>{
            ColoredVertex{position: FixedArray<float, 3>{4, 0, 10}, color: FixedArray<float, 3>{0, 1, 1}},
            ColoredVertex{position: FixedArray<float, 3>{0, 4, 10}, color: FixedArray<float, 3>{0, 1, 1}},
            ColoredVertex{position: FixedArray<float, 3>{1, 1, -5}, color: FixedArray<float, 3>{1, 1, 0}}
        }
    };*/
    std::list<std::shared_ptr<ColoredVertexArray<float>>> triangles1 = load_obj(
        "Data/box.obj",
        LoadMeshConfig{
            .blend_mode = BlendMode::OFF,
            .cull_faces_default = true,
            .cull_faces_alpha = false,
            .occluded_pass = ExternalRenderPassType::LIGHTMAP_DEPTH,
            .occluder_pass = ExternalRenderPassType::LIGHTMAP_DEPTH,
            .aggregate_mode = AggregateMode::NONE,
            .transformation_mode = TransformationMode::ALL,
            .apply_static_lighting = true,
            .laplace_ao_strength = 0.f,
            .werror = true});
    for (auto& o : triangles1) {
        o->physics_material |= (PhysicsMaterial::OBJ_CHASSIS | PhysicsMaterial::ATTR_CONVEX);
    }

    SceneNodeResources scene_node_resources;
    DeleteNodeMutex delete_node_mutex;
    Scene scene{ delete_node_mutex };
    auto rrg = RenderingContextGuard::root(scene_node_resources, "primary_rendering_resources", 16, 0);
    scene_node_resources.add_resource("obj0", std::make_shared<ColoredVertexArrayResource>(triangles0));
    scene_node_resources.add_resource("obj1", std::make_shared<ColoredVertexArrayResource>(triangles1));
    scene_node_resources.add_resource("beacon", load_renderable_obj(
        "Data/box.obj",
        LoadMeshConfig{
            .position = FixedArray<float, 3>{0, 0, 0},
            .rotation = FixedArray<float, 3>{0, 0, 0},
            .scale = FixedArray<float, 3>{0.5, 0.5, 0.5},
            .blend_mode = BlendMode::OFF,
            .cull_faces_default = true,
            .cull_faces_alpha = false,
            .occluded_pass = ExternalRenderPassType::NONE,
            .occluder_pass = ExternalRenderPassType::NONE,
            .aggregate_mode = AggregateMode::NONE,
            .transformation_mode = TransformationMode::ALL,
            .apply_static_lighting = true,
            .laplace_ao_strength = 0.f,
            .werror = true},
        scene_node_resources));
    scene_node_resources.generate_triangle_rays("obj1", 5, {1, 1, 1});
    auto scene_node0 = std::make_unique<SceneNode>();
    auto scene_node1_0 = std::make_unique<SceneNode>();
    auto scene_node1_1 = std::make_unique<SceneNode>();
    auto scene_node1_2 = std::make_unique<SceneNode>();
    auto scene_nodeR = std::make_unique<SceneNode>();
    auto scene_nodeL = std::make_unique<SceneNode>();
    auto& scene_nodeL_p = *scene_nodeL;

    scene_node_resources.instantiate_renderable("obj0", InstantiationOptions{
        .instance_name = "obj0",
        .scene_node = *scene_node0,
        .renderable_resource_filter = RenderableResourceFilter{}});
    scene_node_resources.instantiate_renderable("obj1", InstantiationOptions{
        .instance_name = "obj1_0",
        .scene_node = *scene_node1_0,
        .renderable_resource_filter = RenderableResourceFilter{}});
    scene_node_resources.instantiate_renderable("obj1", InstantiationOptions{
        .instance_name = "obj1_1",
        .scene_node = *scene_node1_1,
        .renderable_resource_filter = RenderableResourceFilter{}});
    scene_node_resources.instantiate_renderable("obj1", InstantiationOptions{
        .instance_name = "obj1_2",
        .scene_node = *scene_node1_2,
        .renderable_resource_filter = RenderableResourceFilter{}});
    if (getenv_default_bool("STACK", false)) {
        scene_node1_1->set_position(FixedArray<double, 3>{0, 4, 0});
        scene_node1_2->set_position(FixedArray<double, 3>{0, 8, 0});
    } else {
        scene_node0->set_rotation(FixedArray<float, 3>{0, 0, 0.001 * M_PI});
        scene_node1_1->set_position(FixedArray<double, 3>{0.1, 4, 0.5});
        scene_node1_2->set_position(FixedArray<double, 3>{0.1, 8, 0.5});
        scene_node1_0->set_rotation(FixedArray<float, 3>{0, 0, 0.1 * M_PI});
        scene_node1_1->set_rotation(FixedArray<float, 3>{0, 0, 0.05 * M_PI});
        scene_node1_2->set_rotation(FixedArray<float, 3>{0, 0, 0.05 * M_PI});
    }

    scene_nodeR->add_child("n0", std::move(scene_node0));
    scene_nodeR->add_child("n1_0", std::move(scene_node1_0));
    scene_nodeR->add_child("n1_1", std::move(scene_node1_1));
    scene_nodeR->add_child("n1_2", std::move(scene_node1_2));
    scene_nodeR->set_position({0.f, -1.f, -40.f});
    scene_nodeL->set_position({0.f, 50.f, -40.f});
    scene_nodeL->set_rotation({-90.f * degrees, 0.f, 0.f});
    SelectedCameras selected_cameras{scene};
    scene_nodeL->add_light(std::make_unique<Light>(Light{
        .resource_suffix = "light_node",
        .shadow_render_pass = ExternalRenderPassType::LIGHTMAP_DEPTH}));
    scene_nodeL->add_light(std::make_unique<Light>(Light{
        .shadow_render_pass = ExternalRenderPassType::NONE}));

    scene.add_root_node("obj", std::move(scene_nodeR));
    scene.add_root_node("follower_camera", std::make_unique<SceneNode>());
    scene.add_root_node("light_node", std::move(scene_nodeL));
    scene.get_node("follower_camera").set_camera(std::make_unique<PerspectiveCamera>(
        PerspectiveCameraConfig(),
        PerspectiveCamera::Postprocessing::ENABLED));
    scene.get_node("light_node").set_camera(std::make_unique<PerspectiveCamera>(
        PerspectiveCameraConfig(),
        PerspectiveCamera::Postprocessing::ENABLED));

    // Must be done when node is already linked to its parents.
    scene.get_node("obj").get_child("n0").set_absolute_movable(rb0.get());
    scene.get_node("obj").get_child("n1_0").set_absolute_movable(rb1_0.get());
    scene.get_node("obj").get_child("n1_1").set_absolute_movable(rb1_1.get());
    scene.get_node("obj").get_child("n1_2").set_absolute_movable(rb1_2.get());

    pe.rigid_bodies_.add_rigid_body(rb0, {triangles0}, {}, CollidableMode::TERRAIN, PhysicsResourceFilter{});
    pe.rigid_bodies_.add_rigid_body(rb1_0, triangles1, {}, CollidableMode::SMALL_MOVING, PhysicsResourceFilter{});
    pe.rigid_bodies_.add_rigid_body(rb1_1, triangles1, {}, CollidableMode::SMALL_MOVING, PhysicsResourceFilter{});
    pe.rigid_bodies_.add_rigid_body(rb1_2, triangles1, {}, CollidableMode::SMALL_MOVING, PhysicsResourceFilter{});

    // Check if the initialization does not change the node positions.
    // Not that only "physics advance time" can change the positions.
    assert_allclose(scene.get_node("obj").get_child("n0").position(), fixed_zeros<double, 3>());
    scene.move(physics_cfg.dt);
    assert_allclose(scene.get_node("obj").get_child("n0").position(), fixed_zeros<double, 3>());

    GravityEfp gefp{ gravity_vector };
    pe.add_external_force_provider(&gefp);

    SetFps physics_set_fps{"Physics FPS: "};
    PhysicsIteration pi{
        scene_node_resources,
        scene,
        pe,
        delete_node_mutex,
        physics_cfg};
    delete_node_mutex.clear_deleter_thread();
    PhysicsLoop pl{
        "Physics",
        pi,
        physics_cfg,
        physics_set_fps,
        is_interactive ? SIZE_MAX : 20};
    if (!is_interactive) {
        pl.join();
        if (unhandled_exceptions_occured()) {
            print_unhandled_exceptions();
            throw std::runtime_error("Unhandled exception occured");
        }
    }

    StandardCameraLogic standard_camera_logic{
        scene,
        selected_cameras,
        delete_node_mutex};
    StandardRenderLogic standard_render_logic{
        scene,
        standard_camera_logic,
        {1.f, 0.f, 1.f},
        ClearMode::COLOR_AND_DEPTH};
    Focuses focuses = {Focus::SCENE};
    ButtonStates button_states;
    CursorStates cursor_states;
    CursorStates scroll_wheel_states;
    FlyingCameraUserClass user_object{
        .button_states = button_states,
        .cursor_states = cursor_states,
        .scroll_wheel_states = scroll_wheel_states,
        .cameras = selected_cameras,
        .focuses = focuses,
        .wire_frame = render_config.wire_frame,
        .depth_test = render_config.depth_test,
        .cull_faces = render_config.cull_faces,
        .delete_node_mutex = delete_node_mutex,
        .physics_set_fps = &physics_set_fps};
    auto flying_camera_logic = std::make_shared<FlyingCameraLogic>(
        render2.glfw_window(),
        button_states,
        scene,
        user_object,
        false,
        false); // false = fly, false = rotate
    auto read_pixels_logic = std::make_shared<ReadPixelsLogic>(standard_render_logic);
    auto lightmap_logic = std::make_shared<LightmapLogic>(
        *read_pixels_logic,
        ExternalRenderPassType::LIGHTMAP_DEPTH,
        scene_nodeL_p,
        "light_node",
        "",    // black_node_name
        true); // with_depth_texture

    UiFocus ui_focus;
    RenderLogics render_logics{delete_node_mutex, ui_focus};
    render_logics.append(nullptr, flying_camera_logic);
    render_logics.append(nullptr, lightmap_logic);
    render_logics.append(nullptr, read_pixels_logic);

    render2.render(
        render_logics,
        SceneGraphConfig());
    if (unhandled_exceptions_occured()) {
        print_unhandled_exceptions();
        throw std::runtime_error("Unhandled exceptions happened");
    }

    if (!is_interactive) {
        Array<float>& rgb = render_results.outputs.at(rsd).rgb;
        if (!rgb.initialized()) {
            throw std::runtime_error("Render output not set");
        }
        draw_nan_masked_rgb(rgb, 0, 1).save_to_file("TestOut/scene_test.png");
    } else {
        pl.stop_and_join();
        if (unhandled_exceptions_occured()) {
            print_unhandled_exceptions();
            throw std::runtime_error("Unhandled exception occured");
        }
    }
}


int main(int argc, char** argv) {
    enable_floating_point_exceptions();

    try {
        test_physics_engine();
    } catch (const std::runtime_error& e) {
        std::cerr << e.what() << std::endl;
        return 1;
    }
    return 0;
}
