#include "Create_Scene_Flat.hpp"
#include "Create_Scene_Rod.hpp"
#include "Create_Scene_Slide.hpp"
#include <Mlib/Env.hpp>
#include <Mlib/Floating_Point_Exceptions.hpp>
#include <Mlib/Geometry/Cameras/Perspective_Camera.hpp>
#include <Mlib/Geometry/Colored_Vertex.hpp>
#include <Mlib/Geometry/Mesh/Load/Load_Mesh_Config.hpp>
#include <Mlib/Geometry/Mesh/Load/Load_Obj.hpp>
#include <Mlib/Geometry/Physics_Material.hpp>
#include <Mlib/Images/Draw_Bmp.hpp>
#include <Mlib/Macro_Executor/Focus.hpp>
#include <Mlib/Math/Fixed_Test.hpp>
#include <Mlib/Math/Pi.hpp>
#include <Mlib/Memory/Destruction_Guard.hpp>
#include <Mlib/Memory/Object_Pool.hpp>
#include <Mlib/Physics/Bullets/Bullet_Property_Db.hpp>
#include <Mlib/Physics/Collision/Collidable_Mode.hpp>
#include <Mlib/Physics/Collision/Power_To_Force.hpp>
#include <Mlib/Physics/Misc/Gravity_Efp.hpp>
#include <Mlib/Physics/Physics_Engine/Physics_Engine.hpp>
#include <Mlib/Physics/Physics_Engine/Physics_Iteration.hpp>
#include <Mlib/Physics/Physics_Engine/Physics_Loop.hpp>
#include <Mlib/Physics/Rigid_Body/Rigid_Body_Vehicle.hpp>
#include <Mlib/Physics/Rigid_Body/Rigid_Primitives.hpp>
#include <Mlib/Physics/Smoke_Generation/Contact_Smoke_Generator.hpp>
#include <Mlib/Physics/Smoke_Generation/Smoke_Particle_Generator.hpp>
#include <Mlib/Physics/Smoke_Generation/Surface_Contact_Db.hpp>
#include <Mlib/Render/Batch_Renderers/Trail_Renderer.hpp>
#include <Mlib/Render/Clear_Wrapper.hpp>
#include <Mlib/Render/Deallocate/Render_Allocator.hpp>
#include <Mlib/Render/Input_Config.hpp>
#include <Mlib/Render/Render.hpp>
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
#include <Mlib/Render/Resource_Managers/Particle_Resources.hpp>
#include <Mlib/Render/Resource_Managers/Rendering_Resources.hpp>
#include <Mlib/Render/Resource_Managers/Trail_Resources.hpp>
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
#include <Mlib/Scene_Graph/Instances/Dynamic_World.hpp>
#include <Mlib/Scene_Graph/Instantiation/Child_Instantiation_Options.hpp>
#include <Mlib/Scene_Graph/Resources/Physics_Resource_Filter.hpp>
#include <Mlib/Scene_Graph/Resources/Renderable_Resource_Filter.hpp>
#include <Mlib/Scene_Graph/Resources/Scene_Node_Resources.hpp>
#include <Mlib/Threads/Realtime_Threads.hpp>
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
    InputConfig input_config;
    FixedTimeSleeper render_sleeper{ 1.f / 60.f };
    SetFps set_fps{
        &render_sleeper,
        std::function<std::chrono::steady_clock::time_point()>(), // simulated_time
        [](){ return false; }                                     // paused
    };
    Render render{
        render_config,
        input_config,
        num_renderings,
        set_fps,
        [is_interactive]() {
            if (is_interactive) {
                return std::chrono::steady_clock::now();
            } else {
                return std::chrono::steady_clock::time_point();
            }
        },
        &render_results };
    ClearWrapperGuard clear_wrapper_guard;

    PhysicsEngineConfig physics_cfg{
        .dt = getenv_default_float("DT", 0.01667f) * seconds,
        .stiction_coefficient = getenv_default_float("FRICTION", 1.f),
        .friction_coefficient = getenv_default_float("FRICTION", 1.f),
        .nsubsteps = getenv_default_size_t("NSUBSTEPS", 20),
        .enable_ridge_map = true
    };
    // SceneNode destructors require that physics engine is destroyed after scene,
    // => Create PhysicsEngine before Scene
    PhysicsEngine pe{ physics_cfg };

    SceneNodeResources scene_node_resources;
    ParticleResources particle_resources;
    TrailResources trail_resources;
    DeleteNodeMutex delete_node_mutex;
    Scene scene{ "main_scene", delete_node_mutex };
    DestructionGuard scene_destruction_guard{[&](){
        scene.shutdown();
    }};
    RenderingResources rendering_resources{
        "primary_rendering_resources",
        16 };
    SurfaceContactDb surface_contact_db;
    BulletPropertyDb bullet_property_db;
    SmokeParticleGenerator smoke_particle_generator{ &rendering_resources, scene_node_resources, scene };
    ContactSmokeGenerator contact_smoke_generator{ smoke_particle_generator };
    TrailRenderer trail_renderer{ trail_resources };
    pe.set_surface_contact_db(surface_contact_db);
    pe.set_contact_smoke_generator(contact_smoke_generator);
    pe.set_trail_renderer(trail_renderer);
    RenderingContext primary_rendering_context{
        .scene_node_resources = scene_node_resources,
        .particle_resources = particle_resources,
        .trail_resources = trail_resources,
        .rendering_resources = rendering_resources,
        .z_order = 0};
    RenderingContextGuard rcg{ primary_rendering_context };

    SelectedCameras selected_cameras{ scene };
    auto scene_name = std::string{getenv_default("SCENE", "flat")};
    if (scene_name == "flat") {
        create_scene_flat(
            scene,
            pe,
            selected_cameras,
            physics_cfg,
            seed);
    } else if (scene_name == "rod") {
        create_scene_rod(
            scene,
            pe,
            selected_cameras);
    } else if (scene_name == "slide") {
        create_scene_slide(
            scene,
            pe,
            selected_cameras);
    } else {
        THROW_OR_ABORT("Unknown scene name");
    }

    GravityEfp gefp;
    pe.add_external_force_provider(gefp);

    RealtimeSleeper physics_sleeper{
        "Physics FPS: ",
        physics_cfg.dt / seconds,
        physics_cfg.max_residual_time / seconds,
        physics_cfg.print_residual_time};
    SetFps physics_set_fps{
        physics_cfg.control_fps
            ? &physics_sleeper
            : nullptr,
        [&]() { return physics_sleeper.simulated_time(); }, // simulated_time
        []() { return false; }                              // paused
    };
    scene_node_resources.register_gravity("world", { 0.f, -9.8f * meters / squared(seconds), 0.f });
    DynamicWorld dynamic_world{ scene_node_resources, "world" };
    PhysicsIteration pi{
        scene_node_resources,
        rendering_resources,
        scene,
        dynamic_world,
        pe,
        delete_node_mutex,
        physics_cfg };
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
        selected_cameras};
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
    UiFocus ui_focus{""};
    RenderLogics render_logics{ui_focus};
    ObjectPool object_pool{ InObjectPoolDestructor::CLEAR };
    auto& flying_camera_logic = object_pool.create<FlyingCameraLogic>(
        CURRENT_SOURCE_LOCATION,
        scene,
        user_object,
        false,  // false = fly
        false); // false = rotate
    auto& read_pixels_logic = object_pool.create<ReadPixelsLogic>(
        CURRENT_SOURCE_LOCATION,
        standard_render_logic,
        button_states,
        ReadPixelsRole::INTERMEDIATE | ReadPixelsRole::SCREENSHOT);
    auto append_lightmap_logic = [&](){
        DanglingRef<SceneNode> light_node = scene.get_node("light_node", DP_LOC);
        // Light without shadow
        light_node->add_light(std::make_unique<Light>(Light{
            .shadow_render_pass = ExternalRenderPassType::NONE}));
        // Light with shadow
        auto light = std::make_shared<Light>(Light{
            .lightmap_color = nullptr,
            .lightmap_depth = nullptr,
            .shadow_render_pass = ExternalRenderPassType::LIGHTMAP_DEPTH});
        light_node->add_light(light);
        auto& lightmap_logic = global_object_pool.create<LightmapLogic>(
            CURRENT_SOURCE_LOCATION,
            rendering_resources,
            read_pixels_logic,
            ExternalRenderPassType::LIGHTMAP_DEPTH,
            light_node,
            light,
            "",                                 // black_node_name
            true,                               // with_depth_texture
            2048,                               // lightmap_width
            2048,                               // lightmap_height
            fixed_zeros<uint32_t, 2>());        // smooth_niterations
        lightmap_logic.on_child_logic_destroy.add([&lightmap_logic]() { global_object_pool.remove(lightmap_logic); }, CURRENT_SOURCE_LOCATION);
        lightmap_logic.on_node_clear.add([&lightmap_logic]() { global_object_pool.remove(lightmap_logic); }, CURRENT_SOURCE_LOCATION);
        render_logics.append({ lightmap_logic, CURRENT_SOURCE_LOCATION }, 0 /* z_order */, CURRENT_SOURCE_LOCATION);
    };

    render_logics.append({ flying_camera_logic, CURRENT_SOURCE_LOCATION }, 0 /* z_order */, CURRENT_SOURCE_LOCATION);
    append_lightmap_logic();
    render_logics.append({ read_pixels_logic, CURRENT_SOURCE_LOCATION }, 0 /* z_order */, CURRENT_SOURCE_LOCATION);
    LambdaRenderLogic lrl{
        [&](
            const LayoutConstraintParameters& lx,
            const LayoutConstraintParameters& ly,
            const RenderConfig& render_config,
            const SceneGraphConfig& scene_graph_config,
            RenderResults* render_results,
            const RenderedSceneDescriptor& frame_id)
        {
            execute_render_allocators();
            scene.wait_for_cleanup();
            render_logics.render_toplevel(lx, ly, render_config, scene_graph_config, render_results, frame_id);
        }
    };

    render.render(
        lrl,
        []() {},
        SceneGraphConfig());
    if (unhandled_exceptions_occured()) {
        print_unhandled_exceptions();
        throw std::runtime_error("Unhandled exceptions happened");
    }

    if (!is_interactive) {
        const Array<float>& rgb = render_results.outputs.at(rsd).rgb;
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
    reserve_realtime_threads(0);
    enable_floating_point_exceptions();

    try {
        auto seed_min = getenv_default_uint("SEED_MIN", 0);
        auto seed_count = getenv_default_uint("SEED_COUNT", 1);
        for (auto seed = seed_min; seed < seed_min + seed_count; ++seed) {
            linfo() << "seed: " << seed;
            test_physics_engine(seed);
        }
    } catch (const std::runtime_error& e) {
        lerr() << e.what();
        return 1;
    }
    return 0;
}
