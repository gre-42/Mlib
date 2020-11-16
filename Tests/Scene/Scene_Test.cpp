#include <Mlib/Env.hpp>
#include <Mlib/Geometry/Mesh/Load_Obj.hpp>
#include <Mlib/Images/Draw_Bmp.hpp>
#include <Mlib/Math/Pi.hpp>
#include <Mlib/Physics/Misc/Gravity_Efp.hpp>
#include <Mlib/Physics/Misc/Rigid_Body.hpp>
#include <Mlib/Physics/Misc/Rigid_Primitives.hpp>
#include <Mlib/Physics/Physics_Engine.hpp>
#include <Mlib/Physics/Physics_Loop.hpp>
#include <Mlib/Physics/Power_To_Force.hpp>
#include <Mlib/Render/Cameras/Generic_Camera.hpp>
#include <Mlib/Render/Render2.hpp>
#include <Mlib/Render/Render_Logics/Flying_Camera_Logic.hpp>
#include <Mlib/Render/Render_Logics/Lightmap_Logic.hpp>
#include <Mlib/Render/Render_Logics/Read_Pixels_Logic.hpp>
#include <Mlib/Render/Render_Logics/Render_Logics.hpp>
#include <Mlib/Render/Render_Logics/Standard_Camera_Logic.hpp>
#include <Mlib/Render/Render_Logics/Standard_Render_Logic.hpp>
#include <Mlib/Render/Render_Results.hpp>
#include <Mlib/Render/Renderables/Renderable_Colored_Vertex_Array.hpp>
#include <Mlib/Render/Renderables/Renderable_Obj_File.hpp>
#include <Mlib/Render/Rendering_Resources.hpp>
#include <Mlib/Render/Selected_Cameras.hpp>
#include <Mlib/Render/Ui/Button_States.hpp>
#include <Mlib/Scene_Graph/Camera_Config.hpp>
#include <Mlib/Scene_Graph/Scene.hpp>
#include <Mlib/Scene_Graph/Scene_Node_Resources.hpp>
#include <Mlib/Set_Fps.hpp>
#include <atomic>
#include <fenv.h>
#include <thread>

using namespace Mlib;

void test_physics_engine() {
    size_t num_renderings = SIZE_MAX;
    RenderResults render_results;
    RenderedSceneDescriptor rsd{external_render_pass: {ExternalRenderPass::STANDARD_WITH_POSTPROCESSING, ""}, time_id: 0, light_resource_id: 0};
    bool is_interactive = getenv_default_bool("PHYSICS_INTERACTIVE", false);
    if (!is_interactive) {
        render_results.outputs[rsd] = Array<float>{};
    }
    RenderConfig render_config{
        nsamples_msaa: 1,
        cull_faces: true,
        wire_frame: false,
        window_title: "Physics test"};
    Render2 render2{
        num_renderings,
        &render_results,
        render_config};

    PhysicsEngineConfig physics_cfg{
        .dt = getenv_default_float("DT", 0.01667),
        .resolve_collision_type = getenv_default_bool("SEQUENTIAL_PULSES", false)
            ? ResolveCollisionType::SEQUENTIAL_PULSES
            : ResolveCollisionType::PENALTY,
        .oversampling = getenv_default_size_t("OVERSAMPLING", 20)};
    // SceneNode destructors require that physics engine is destroyed after scene,
    // => Create PhysicsEngine before Scene
    PhysicsEngine pe{physics_cfg};

    std::shared_ptr<RigidBody> rb0 = rigid_cuboid(pe.rigid_bodies_, INFINITY, {1, 2, 3});
    std::shared_ptr<RigidBody> rb1_0 = rigid_cuboid(pe.rigid_bodies_, 3, {2, 3, 4});
    std::shared_ptr<RigidBody> rb1_1 = rigid_cuboid(pe.rigid_bodies_, 3, {2, 3, 4});
    std::shared_ptr<RigidBody> rb1_2 = rigid_cuboid(pe.rigid_bodies_, 3, {2, 3, 4});

    std::vector<FixedArray<ColoredVertex, 3>> triangles0_raw{
        FixedArray<ColoredVertex, 3>{
            ColoredVertex{position: {-10, -2, +10}, color: {0, 0, 1}, normal: {0, 1, 0}},
            ColoredVertex{position: {+10, -2, -10}, color: {0, 1, 0}, normal: {0, 1, 0}},
            ColoredVertex{position: {-10, -2, -10}, color: {1, 0, 0}, normal: {0, 1, 0}}},
        FixedArray<ColoredVertex, 3>{
            ColoredVertex{position: {+10, -2, -10}, color: {0, 0, 1}, normal: {0, 1, 0}},
            ColoredVertex{position: {-10, -2, +10}, color: {0, 1, 0}, normal: {0, 1, 0}},
            ColoredVertex{position: {+10, -2, +10}, color: {1, 0, 0}, normal: {0, 1, 0}}}
    };
    auto triangles0 = std::make_shared<ColoredVertexArray>(
        "test_physics_engine",
        Material{
            occluded_type: OccludedType::LIGHT_MAP_DEPTH,
            occluder_type: OccluderType::BLACK},
        std::move(triangles0_raw),
        std::move(std::vector<FixedArray<ColoredVertex, 2>>()));

    /*std::vector<FixedArray<ColoredVertex, 3>> triangles1{
        FixedArray<ColoredVertex, 3>{
            ColoredVertex{position: FixedArray<float, 3>{4, 0, 10}, color: FixedArray<float, 3>{0, 1, 1}},
            ColoredVertex{position: FixedArray<float, 3>{0, 4, 10}, color: FixedArray<float, 3>{0, 1, 1}},
            ColoredVertex{position: FixedArray<float, 3>{1, 1, -5}, color: FixedArray<float, 3>{1, 1, 0}}
        }
    };*/
    std::list<std::shared_ptr<ColoredVertexArray>> triangles1 = load_obj(
        "Data/box.obj",
        true,                            // is_small
        BlendMode::OFF,
        false,                           // blend_cull_faces
        OccludedType::LIGHT_MAP_DEPTH,
        OccluderType::BLACK,
        true,                            // occluded_by_black
        AggregateMode::OFF,
        true,                            // apply_static_lighting
        true);                           // werror

    SceneNodeResources scene_node_resources;
    Scene scene;
    RenderingResources rendering_resources;
    scene_node_resources.add_resource("obj0", std::make_shared<RenderableColoredVertexArray>(triangles0, nullptr, &rendering_resources));
    scene_node_resources.add_resource("obj1", std::make_shared<RenderableColoredVertexArray>(triangles1, nullptr, &rendering_resources));
    scene_node_resources.add_resource("beacon", std::make_shared<RenderableObjFile>(
        "Data/box.obj",
        FixedArray<float, 3>{0, 0, 0},        // position
        FixedArray<float, 3>{0, 0, 0},        // rotation
        FixedArray<float, 3>{0.5, 0.5, 0.5},  // scale
        &rendering_resources,
        true,                                 // is_small
        BlendMode::OFF,
        false,                                // blend_cull_faces
        OccludedType::OFF,
        OccluderType::OFF,
        true,                                 // occluded_by_black
        AggregateMode::OFF,
        true,                                 // apply_static_lighting
        true                                  // werror
    ));
    scene_node_resources.generate_triangle_rays("obj1", 5, {1, 1, 1});
    auto scene_node0 = new SceneNode;
    auto scene_node1_0 = new SceneNode;
    auto scene_node1_1 = new SceneNode;
    auto scene_node1_2 = new SceneNode;
    auto scene_nodeR = new SceneNode;
    auto scene_nodeL = new SceneNode;

    scene_node_resources.instantiate_renderable("obj0", "obj0", *scene_node0, SceneNodeResourceFilter{});
    scene_node_resources.instantiate_renderable("obj1", "obj1_0", *scene_node1_0, SceneNodeResourceFilter{});
    scene_node_resources.instantiate_renderable("obj1", "obj1_1", *scene_node1_1, SceneNodeResourceFilter{});
    scene_node_resources.instantiate_renderable("obj1", "obj1_2", *scene_node1_2, SceneNodeResourceFilter{});
    if (getenv_default_bool("STACK", false)) {
        scene_node1_1->set_position(FixedArray<float, 3>{0, 4, 0});
        scene_node1_2->set_position(FixedArray<float, 3>{0, 8, 0});
    } else {
        scene_node0->set_rotation(FixedArray<float, 3>{0, 0, 0.001 * M_PI});
        scene_node1_1->set_position(FixedArray<float, 3>{0.1, 4, 0.5});
        scene_node1_2->set_position(FixedArray<float, 3>{0.1, 8, 0.5});
        scene_node1_0->set_rotation(FixedArray<float, 3>{0, 0, 0.1 * M_PI});
        scene_node1_1->set_rotation(FixedArray<float, 3>{0, 0, 0.05 * M_PI});
        scene_node1_2->set_rotation(FixedArray<float, 3>{0, 0, 0.05 * M_PI});
    }

    scene_nodeR->add_child("n0", scene_node0);
    scene_nodeR->add_child("n1_0", scene_node1_0);
    scene_nodeR->add_child("n1_1", scene_node1_1);
    scene_nodeR->add_child("n1_2", scene_node1_2);
    scene_node0->set_parent(scene_nodeR);
    scene_node1_0->set_parent(scene_nodeR);
    scene_node1_1->set_parent(scene_nodeR);
    scene_node1_2->set_parent(scene_nodeR);
    scene_nodeR->set_position({0.f, -1.f, -40.f});
    scene_nodeL->set_position({0.f, 50.f, -40.f});
    scene_nodeL->set_rotation({-90.f * M_PI / 180.f, 0.f, 0.f});
    SelectedCameras selected_cameras;
    Light* shadow_light = new Light{
        resource_index: selected_cameras.add_light_node("light_node"),
        only_black: false,
        shadow: true};
    scene_nodeL->add_light(shadow_light);
    scene_nodeL->add_light(new Light{
        resource_index: 1234,
        only_black: false,
        shadow: false});

    scene.add_root_node("obj", scene_nodeR);
    scene.add_root_node("follower_camera", new SceneNode);
    scene.add_root_node("light_node", scene_nodeL);
    scene.get_node("follower_camera")->set_camera(std::make_shared<GenericCamera>(CameraConfig{}, GenericCamera::Mode::PERSPECTIVE));
    scene.get_node("light_node")->set_camera(std::make_shared<GenericCamera>(CameraConfig{}, GenericCamera::Mode::PERSPECTIVE));

    // Must be done when node is already linked to its parents.
    scene_node0->set_absolute_movable(rb0.get());
    scene_node1_0->set_absolute_movable(rb1_0.get());
    scene_node1_1->set_absolute_movable(rb1_1.get());
    scene_node1_2->set_absolute_movable(rb1_2.get());

    pe.rigid_bodies_.add_rigid_body(rb0, {triangles0}, {});
    pe.rigid_bodies_.add_rigid_body(rb1_0, triangles1, {});
    pe.rigid_bodies_.add_rigid_body(rb1_1, triangles1, {});
    pe.rigid_bodies_.add_rigid_body(rb1_2, triangles1, {});

    assert_allclose(scene_node1_0->position().to_array(), fixed_zeros<float, 3>().to_array());
    scene.move();
    assert_allclose(scene_node1_0->position().to_array(), fixed_zeros<float, 3>().to_array());

    GravityEfp gefp({0, -9.8, 0});
    pe.add_external_force_provider(&gefp);

    std::shared_mutex mutex;
    SetFps physics_set_fps;
    PhysicsLoop pl{
        scene_node_resources,
        scene,
        pe,
        mutex,
        physics_cfg,
        physics_set_fps,
        is_interactive ? SIZE_MAX : 20};
    if (!is_interactive) {
        pl.join();
    }

    StandardCameraLogic standard_camera_logic{scene, selected_cameras};
    StandardRenderLogic standard_render_logic{scene, standard_camera_logic};
    std::list<Focus> focus = {Focus::SCENE};
    ButtonStates button_states;
    FlyingCameraUserClass user_object{
        button_states: button_states,
        cameras: selected_cameras,
        focus: focus,
        physics_set_fps: &physics_set_fps};
    auto flying_camera_logic = std::make_shared<FlyingCameraLogic>(
        render2.window(),
        button_states,
        scene,
        user_object,
        false,
        false); // false = fly, false = rotate
    auto read_pixels_logic = std::make_shared<ReadPixelsLogic>(standard_render_logic);
    auto lightmap_logic = std::make_shared<LightmapLogic>(
        *read_pixels_logic,
        rendering_resources,
        LightmapUpdateCycle::ALWAYS,
        shadow_light->resource_index,
        "",    // black_node_name
        true); // with_depth_texture

    RenderLogics render_logics;
    render_logics.append(nullptr, flying_camera_logic);
    render_logics.append(nullptr, lightmap_logic);
    render_logics.append(nullptr, read_pixels_logic);

    render2(
        render_logics,
        mutex,
        SceneGraphConfig{});

    if (!is_interactive) {
        draw_nan_masked_rgb(render_results.outputs.at(rsd), 0, 1).save_to_file("TestOut/scene_test.ppm");
    } else {
        pl.stop_and_join();
    }
}

int main(int argc, const char** argv) {
    #ifndef __MINGW32__
    feenableexcept(FE_INVALID);
    #endif

    test_physics_engine();
    return 0;
}
