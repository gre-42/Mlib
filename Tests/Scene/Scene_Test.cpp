#include "Create_Scene_Flat.hpp"
#include "Create_Scene_Rod.hpp"
#include <Mlib/Env.hpp>
#include <Mlib/Floating_Point_Exceptions.hpp>
#include <Mlib/Geometry/Cameras/Perspective_Camera.hpp>
#include <Mlib/Geometry/Colored_Vertex.hpp>
#include <Mlib/Geometry/Mesh/Load/Load_Mesh_Config.hpp>
#include <Mlib/Geometry/Mesh/Load/Load_Obj.hpp>
#include <Mlib/Geometry/Physics_Material.hpp>
#include <Mlib/Images/Draw_Bmp.hpp>
#include <Mlib/Math/Fixed_Test.hpp>
#include <Mlib/Math/Pi.hpp>
#include <Mlib/Memory/Destruction_Guard.hpp>
#include <Mlib/Physics/Collision/Collidable_Mode.hpp>
#include <Mlib/Physics/Collision/Power_To_Force.hpp>
#include <Mlib/Physics/Gravity.hpp>
#include <Mlib/Physics/Misc/Gravity_Efp.hpp>
#include <Mlib/Physics/Physics_Engine/Physics_Engine.hpp>
#include <Mlib/Physics/Physics_Engine/Physics_Iteration.hpp>
#include <Mlib/Physics/Physics_Engine/Physics_Loop.hpp>
#include <Mlib/Physics/Rigid_Body/Rigid_Body_Vehicle.hpp>
#include <Mlib/Physics/Rigid_Body/Rigid_Primitives.hpp>
#include <Mlib/Physics/Smoke_Generation/Contact_Smoke_Generator.hpp>
#include <Mlib/Physics/Smoke_Generation/Smoke_Particle_Generator.hpp>
#include <Mlib/Physics/Smoke_Generation/Surface_Contact_Db.hpp>
#include <Mlib/Render/Particle_Resources.hpp>
#include <Mlib/Render/Render2.hpp>
#include <Mlib/Render/Render_Config.hpp>
#include <Mlib/Render/Render_Logics/Clear_Mode.hpp>
#include <Mlib/Render/Render_Logics/Flying_Camera_Logic.hpp>
#include <Mlib/Render/Render_Logics/Lambda_Render_Logic.hpp>
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
#include <Mlib/Render/Selected_Cameras/Selected_Cameras.hpp>
#include <Mlib/Render/Ui/Button_States.hpp>
#include <Mlib/Render/Ui/Cursor_States.hpp>
#include <Mlib/Scene_Graph/Containers/Scene.hpp>
#include <Mlib/Scene_Graph/Delete_Node_Mutex.hpp>
#include <Mlib/Scene_Graph/Elements/Absolute_Movable_Setter.hpp>
#include <Mlib/Scene_Graph/Elements/Light.hpp>
#include <Mlib/Scene_Graph/Elements/Scene_Node.hpp>
#include <Mlib/Scene_Graph/Focus.hpp>
#include <Mlib/Scene_Graph/Instantiation_Options.hpp>
#include <Mlib/Scene_Graph/Resources/Physics_Resource_Filter.hpp>
#include <Mlib/Scene_Graph/Resources/Renderable_Resource_Filter.hpp>
#include <Mlib/Scene_Graph/Resources/Scene_Node_Resources.hpp>
#include <Mlib/Threads/Termination_Manager.hpp>
#include <Mlib/Threads/Thread_Affinity.hpp>
#include <Mlib/Time/Fps/Fixed_Time_Sleeper.hpp>
#include <Mlib/Time/Fps/Realtime_Sleeper.hpp>
#include <Mlib/Time/Fps/Set_Fps.hpp>
#include <atomic>
#include <thread>

using namespace Mlib;

void test_physics_engine(unsigned int seed) {
    std::atomic_size_t num_renderings = getenv_default_size_t("NUM_RENDERINGS", SIZE_MAX);
    RenderResults render_results;
    RenderedSceneDescriptor rsd;
    bool is_interactive = getenv_default_bool("PHYSICS_INTERACTIVE", false);
    if (!is_interactive) {
        render_results.outputs[rsd] = {
            .width = 640,
            .height = 480
        };
    }
    RenderConfig render_config{
        .nsamples_msaa = 1,
        .cull_faces = BoolRenderOption::UNCHANGED,
        .wire_frame = BoolRenderOption::UNCHANGED,
        .window_title = "Physics test",
        .show_mouse_cursor = true};
    FixedTimeSleeper render_sleeper{ 1.f / 60.f };
    SetFps set_fps{render_sleeper};
    Render2 render2{
        render_config,
        num_renderings,
        set_fps,
        &render_results};

    PhysicsEngineConfig physics_cfg{
        .dt = getenv_default_float("DT", 0.01667f) * s,
        .oversampling = getenv_default_size_t("OVERSAMPLING", 20)};
    // SceneNode destructors require that physics engine is destroyed after scene,
    // => Create PhysicsEngine before Scene
    PhysicsEngine pe{physics_cfg};

    SceneNodeResources scene_node_resources;
    ParticleResources particle_resources;
    DeleteNodeMutex delete_node_mutex;
    Scene scene{ delete_node_mutex };
    DestructionGuard scene_destruction_guard{[&](){
        std::scoped_lock lock{ delete_node_mutex };
        scene.shutdown();
    }};
    SurfaceContactDb surface_contact_db;
    SmokeParticleGenerator smoke_particle_generator{scene, scene_node_resources};
    ContactSmokeGenerator contact_smoke_generator{surface_contact_db, smoke_particle_generator};
    pe.set_contact_smoke_generator(contact_smoke_generator);
    auto rrg = RenderingContextGuard::root(
        scene_node_resources,
        particle_resources,
        "primary_rendering_resources",
        16,
        0);

    SelectedCameras selected_cameras{scene};
    auto scene_name = std::string{getenv_default("SCENE", "flat")};
    if (scene_name == "flat") {
        create_scene_flat(
            scene_node_resources,
            scene,
            pe,
            selected_cameras,
            physics_cfg,
            seed);
    } else if (scene_name == "rod") {
        create_scene_rod(
            scene_node_resources,
            scene,
            pe,
            selected_cameras);
    } else {
        THROW_OR_ABORT("Unknown scene name");
    }

    GravityEfp gefp{ gravity_vector };
    pe.add_external_force_provider(gefp);

    RealtimeSleeper physics_sleeper{
        "Physics FPS: ",
        physics_cfg.dt / s,
        physics_cfg.max_residual_time / s,
        physics_cfg.control_fps,
        physics_cfg.print_residual_time};
    SetFps physics_set_fps{physics_sleeper};
    PhysicsIteration pi{
        scene_node_resources,
        scene,
        pe,
        delete_node_mutex,
        physics_cfg};
    delete_node_mutex.clear_deleter_thread();
    PhysicsLoop pl{
        "Physics",
        ThreadAffinity::POOL,
        pi,
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
    ButtonStates button_states;
    CursorStates cursor_states;
    CursorStates scroll_wheel_states;
    FlyingCameraUserClass user_object{
        .button_states = button_states,
        .cursor_states = cursor_states,
        .scroll_wheel_states = scroll_wheel_states,
        .cameras = selected_cameras,
        .wire_frame = render_config.wire_frame,
        .depth_test = render_config.depth_test,
        .cull_faces = render_config.cull_faces,
        .delete_node_mutex = delete_node_mutex,
        .physics_set_fps = &physics_set_fps};
    UiFocus ui_focus;
    RenderLogics render_logics{ui_focus};
    auto flying_camera_logic = std::make_shared<FlyingCameraLogic>(
        scene,
        user_object,
        false,  // false = fly
        false); // false = rotate
    auto read_pixels_logic = std::make_shared<ReadPixelsLogic>(standard_render_logic);
    auto append_lightmap_logic = [&](){
        std::scoped_lock lock{delete_node_mutex};
        DanglingRef<SceneNode> light_node = scene.get_node("light_node", DP_LOC);
        auto lightmap_logic = std::make_shared<LightmapLogic>(
            *read_pixels_logic,
            ExternalRenderPassType::LIGHTMAP_DEPTH,
            light_node,
            "light_node",
            "",     // black_node_name
            true,   // with_depth_texture
            2048,   // lightmap_width
            2048);  // lightmap_height
        render_logics.append(light_node.ptr(), lightmap_logic);
    };

    render_logics.append(nullptr, flying_camera_logic);
    append_lightmap_logic();
    render_logics.append(nullptr, read_pixels_logic);
    LambdaRenderLogic lrl{
        [&delete_node_mutex, &render_logics](
            const LayoutConstraintParameters& lx,
            const LayoutConstraintParameters& ly,
            const RenderConfig& render_config,
            const SceneGraphConfig& scene_graph_config,
            RenderResults* render_results,
            const RenderedSceneDescriptor& frame_id)
        {
            std::scoped_lock lock{delete_node_mutex};
            render_logics.render(lx, ly, render_config, scene_graph_config, render_results, frame_id);
        }
    };

    render2.render(
        lrl,
        []() {},
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
        unsigned int seed_min = getenv_default_uint("SEED_MIN", 0);
        unsigned int seed_count = getenv_default_uint("SEED_COUNT", 1);
        for (unsigned int seed = seed_min; seed < seed_min + seed_count; ++seed) {
            linfo() << "seed: " << seed;
            test_physics_engine(seed);
        }
    } catch (const std::runtime_error& e) {
        std::cerr << e.what() << std::endl;
        return 1;
    }
    return 0;
}
