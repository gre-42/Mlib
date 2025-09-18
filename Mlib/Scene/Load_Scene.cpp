#include "Load_Scene.hpp"
#include <Mlib/Macro_Executor/Json_Macro_Arguments.hpp>
#include <Mlib/Macro_Executor/Macro_Line_Executor.hpp>
#include <Mlib/Scene/Json_User_Function_Args.hpp>
#include <Mlib/Scene/Json_User_Function_Args.hpp>
#include <Mlib/Scene/Load_Scene_Funcs.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Containers/Add_Ac_Loader.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Containers/Load_Asset_Manifests.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Containers/Load_Replacement_Parameters.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Containers/Update_Gallery.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Create_Tap_Button.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Instances/Ai/Create_Destination_Reached_Ai.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Instances/Ai/Create_Drive_Or_Walk_Ai.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Instances/Ai/Create_Missile_Ai.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Instances/Ai/Create_Vehicle_Follower_Ai.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Instances/Cameras/Create_Ortho_Camera.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Instances/Cameras/Create_Perspective_Camera.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Instances/Cameras/Fit_Canvas_To_Renderables.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Instances/Cameras/Set_Camera.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Instances/Cameras/Set_Camera_Cycle.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Instances/Controllers/Create_Avatar_As_Avatar_Controller.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Instances/Controllers/Create_Avatar_As_Car_Controller.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Instances/Controllers/Create_Car_Controller.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Instances/Controllers/Create_Heli_Controller.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Instances/Controllers/Create_Missile_Controller.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Instances/Controllers/Create_Plane_As_Car_Controller.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Instances/Controllers/Create_Plane_Controller.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Instances/Controllers/Create_Tank_Controller.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Instances/Define_Winner_Conditionals.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Instances/Execute_In_Physics_Thread.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Instances/Hud/Console_Log.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Instances/Hud/Create_Hud_Opponent_Tracker_Logic.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Instances/Hud/Create_Hud_Opponent_Zoom_Logic.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Instances/Hud/Create_Hud_Target_Point_Logic.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Instances/Hud/Create_Visual_Node_Status.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Instances/Hud/Create_Visual_Player_Bullet_Count.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Instances/Hud/Create_Visual_Player_Status.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Instances/Hud/Fill_Pixel_Region_With_Texture.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Instances/Hud/Minimap.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Instances/Hud/Scene_To_Pixel_Region.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Instances/Hud/Scene_To_Texture.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Instances/Hud/Visual_Node_Status_3rd.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Instances/Instantiate_Grass.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Instances/Invalidate_Aggregate_Renderers.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Instances/Inventory/Set_Desired_Weapon.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Instances/Inventory/Set_Inventory_Capacity.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Instances/Key_Bindings/Create_Abs_Idle_Key_Binding.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Instances/Key_Bindings/Create_Abs_Key_Binding.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Instances/Key_Bindings/Create_Avatar_Controller_Idle_Binding.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Instances/Key_Bindings/Create_Avatar_Controller_Key_Binding.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Instances/Key_Bindings/Create_Car_Controller_Idle_Binding.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Instances/Key_Bindings/Create_Car_Controller_Key_Binding.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Instances/Key_Bindings/Create_Gun_Key_Binding.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Instances/Key_Bindings/Create_Plane_Controller_Idle_Binding.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Instances/Key_Bindings/Create_Plane_Controller_Key_Binding.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Instances/Key_Bindings/Create_Print_Camera_Node_Info_Key_Binding.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Instances/Key_Bindings/Create_Rel_Key_Binding.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Instances/Key_Bindings/Create_Rel_Key_Binding_Tripod.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Instances/Key_Bindings/Create_Weapon_Cycle_Key_Binding.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Instances/Lights/Create_Light_Only_Shadow.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Instances/Lights/Create_Light_With_Shadow.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Instances/Lights/Create_Light_Without_Shadow.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Instances/Lights/Set_Soft_Light.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Instances/Nodes/Clear_Renderable_Instance.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Instances/Nodes/Create_Copy_Rotation.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Instances/Nodes/Create_Keep_Offset_From_Camera.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Instances/Nodes/Create_Keep_Offset_From_Movable.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Instances/Nodes/Create_Relative_Transformer.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Instances/Nodes/Delete_Node.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Instances/Nodes/Delete_Root_Node.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Instances/Nodes/Delete_Root_Nodes.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Instances/Nodes/Look_At_Node.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Instances/Nodes/Move_Node_To_Bvh.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Instances/Nodes/Set_Node_Bone.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Instances/Nodes/Set_Node_Rotation.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Instances/Nodes/Try_Delete_Node.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Instances/Nodes/Try_Delete_Root_Node.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Instances/Players/Create_Player.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Instances/Players/Load_Players.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Instances/Players/Player_Change_Aiming_Gun.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Instances/Players/Player_Set_Aiming_Gun.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Instances/Players/Player_Set_Behavior.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Instances/Players/Player_Set_Can_Aim.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Instances/Players/Player_Set_Can_Drive.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Instances/Players/Player_Set_Can_Select_Opponent.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Instances/Players/Player_Set_Can_Select_Weapon.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Instances/Players/Player_Set_Can_Shoot.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Instances/Players/Player_Set_Playback_Waypoints.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Instances/Players/Player_Set_Vehicle_Control_Parameters.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Instances/Players/Player_Set_Waypoint.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Instances/Players/Respawn_All_Players.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Instances/Players/Set_Race_Identifier_And_Reload_History.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Instances/Players/Set_Spawn_Points.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Instances/Players/Set_Way_Points.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Instances/Players/Start_Race.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Instances/Players/Team_Set_Waypoint.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Instances/Preload.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Instances/Render/Clear_Skybox.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Instances/Render/Root_Renderable_Instances.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Instances/Render/Set_Background_Color.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Instances/Render/Set_Dirtmap.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Instances/Render/Set_Skybox.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Instances/Reset_Supply_Depot_Cooldowns.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Instances/Save_To_Obj_File.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Instances/Set_Externals_Creator.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Instances/Set_Objective.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Instances/Spawners/Create_Spawner.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Instances/Spawners/Spawner_Set_Nodes.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Instances/Spawners/Spawner_Set_Player.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Instances/Spawners/Spawner_Set_Respawn_Cooldown_Time.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Instances/Track/Playback_Track.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Instances/Track/Playback_Winner_Track.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Instances/Track/Record_Track.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Instances/Track/Record_Track_Gpx.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Instances/Ui/Clear_Parameters.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Instances/Ui/Controls.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Instances/Ui/Ui_Exhibit.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Instances/Vehicles/Add_To_Inventory.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Instances/Vehicles/Add_Weapon_To_Cycle.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Instances/Vehicles/Burn_In.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Instances/Vehicles/Connect_Trailer.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Instances/Vehicles/Create_Aim_At.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Instances/Vehicles/Create_Crash.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Instances/Vehicles/Create_Damageable.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Instances/Vehicles/Create_Delta_Engine.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Instances/Vehicles/Create_Engine.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Instances/Vehicles/Create_Gun.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Instances/Vehicles/Create_Rotor.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Instances/Vehicles/Create_Trailer_Node.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Instances/Vehicles/Create_Weapon_Cycle.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Instances/Vehicles/Create_Wheel.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Instances/Vehicles/Create_Wing.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Instances/Vehicles/Create_Yaw_Pitch_Lookat_Nodes.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Instances/Vehicles/Follow_Node.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Instances/Vehicles/Set_Actor_Task.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Instances/Vehicles/Set_Bevel_Box_Surface_Normal.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Instances/Vehicles/Set_Capsule_Surface_Normal.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Instances/Vehicles/Set_RigidBody_Grind_Point.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Instances/Vehicles/Set_Rigid_Body_Align_To_Surface_Relaxation.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Instances/Vehicles/Set_Rigid_Body_Door_Distance.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Instances/Vehicles/Set_Rigid_Body_Revert_Surface_Power_Threshold.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Instances/Vehicles/Set_Rigid_Body_Target.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Instances/Vehicles/Set_Sliding_Normal_Modifier.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Render/Set_Render_Fps.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Render/Set_Textures_Lazy.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Resources/Add_Audio.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Resources/Add_Audio_Sequence.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Resources/Add_Blend_Map_Texture.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Resources/Add_Bvh_Resource.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Resources/Add_Companion_Renderable.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Resources/Add_Foliage_Resource.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Resources/Add_Texture_Atlas.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Resources/Animatable_Billboards.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Resources/Animatable_Trails.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Resources/Animated_Billboards.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Resources/Animated_Trails.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Resources/Cleanup_Mesh.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Resources/Convex_Decompose_Terrain.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Resources/Create_Binary_X_Resource.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Resources/Create_Blending_X_Resource.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Resources/Create_Grid_Resource.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Resources/Create_Square_Resource.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Resources/Delete_Mesh.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Resources/Downsample.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Resources/Gen_Contour_Edges.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Resources/Gen_Grind_Lines.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Resources/Gen_Instances.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Resources/Gen_Ray.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Resources/Gen_Triangle_Rays.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Resources/Import_Bone_Weights.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Resources/Load_Osm_Resource.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Resources/Merge_Meshes.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Resources/Merge_Textures.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Resources/Modify_Physics_Material_Tags.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Resources/Modify_Rendering_Material.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Resources/Obj_Resource.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Resources/Print_Resource.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Resources/Repeat.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Resources/Save_Texture_Array_Png.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Resources/Save_Texture_Png.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Resources/Set_Animated_Dynamic_Light_Properties.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Resources/Set_Bounds.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Resources/Set_Bullet_Properties.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Resources/Set_Constant_Dynamic_Light_Properties.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Resources/Set_Surface_Contact_Info.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Resources/Shade_Auto.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Resources/Smoothen_Edges.hpp>
#include <Mlib/Scene/Load_Scene_Functions/World/Register_Gravity.hpp>
#include <Mlib/Scene/Load_Scene_Functions/World/Register_Wind.hpp>
#include <Mlib/Scene/Physics_Scenes.hpp>
#include <Mlib/Scene/Renderable_Scenes.hpp>

using namespace Mlib;
using namespace LoadSceneFuncs;

LoadScene::LoadScene(
    const std::list<std::string>* search_path,
    const std::string& script_filename,
    ThreadSafeString& next_scene_filename,
    NotifyingJsonMacroArguments& external_json_macro_arguments,
    std::atomic_size_t& num_renderings,
    RealtimeDependentFps& render_set_fps,
    bool verbose,
    SurfaceContactDb& surface_contact_db,
    BulletPropertyDb& bullet_property_db,
    DynamicLightDb& dynamic_light_db,
    SceneConfig& scene_config,
    ButtonStates& button_states,
    CursorStates& cursor_states,
    CursorStates& scroll_wheel_states,
    VerboseVector<ButtonPress>& confirm_button_press,
    LockableKeyConfigurations& key_configurations,
    LockableKeyDescriptions& key_descriptions,
    UiFocuses& ui_focuses,
    LayoutConstraints& layout_constraints,
    RenderLogicGallery& gallery,
    AssetReferences& asset_references,
    Translators& translators,
    PhysicsScenes& physics_scenes,
    RenderableScenes& renderable_scenes,
    WindowLogic& window_logic,
    const std::function<void()>& exit)
    : json_user_function_{ [&](
        const std::string& context,
        const MacroLineExecutor& macro_line_executor,
        const std::string& name,
        const JsonMacroArguments& arguments,
        JsonMacroArguments* local_json_macro_arguments)
    {
        auto physics_scene = [&]() -> PhysicsScene& {
            return physics_scenes[context];
        };
        auto renderable_scene = [&]() -> RenderableScene& {
            return renderable_scenes[context];
        };
        LoadSceneJsonUserFunctionArgs args{
            .name = name,
            .arguments = arguments,
            .physics_scene = physics_scene,
            .renderable_scene = renderable_scene,
            .macro_line_executor = macro_line_executor,
            .external_json_macro_arguments = external_json_macro_arguments,
            .local_json_macro_arguments = local_json_macro_arguments,
            .surface_contact_db = surface_contact_db,
            .bullet_property_db = bullet_property_db,
            .dynamic_light_db = dynamic_light_db,
            .scene_config = scene_config,
            .button_states = button_states,
            .cursor_states = cursor_states,
            .scroll_wheel_states = scroll_wheel_states,
            .confirm_button_press = confirm_button_press,
            .key_configurations = key_configurations,
            .key_descriptions = key_descriptions,
            .ui_focuses = ui_focuses,
            .layout_constraints = layout_constraints,
            .num_renderings = num_renderings,
            .render_set_fps = render_set_fps,
            .script_filename = script_filename,
            .next_scene_filename = next_scene_filename,
            .gallery = gallery,
            .asset_references = asset_references,
            .translators = translators,
            .physics_scenes = physics_scenes,
            .renderable_scenes = renderable_scenes,
            .window_logic = window_logic,
            .exit = exit};
        auto& funcs = json_user_functions();
        auto it = funcs.find(args.name);
        if (it == funcs.end()) {
            return false;
        }
        it->second(args);
        return true;
    }},
    macro_line_executor_{
        macro_file_executor_,
        script_filename,
        search_path,
        json_user_function_,
        "no_scene_specified",
        nlohmann::json::object(),
        external_json_macro_arguments,
        asset_references,
        verbose},
    focus_finalizer_{ ui_focuses, macro_line_executor_ }
{
    static struct RegisterJsonUserFunctions {
        RegisterJsonUserFunctions() {
            // Containers
            register_json_user_function(AddAcLoader::key, AddAcLoader::json_user_function);
            register_json_user_function(LoadAssetManifests::key, LoadAssetManifests::json_user_function);
            register_json_user_function(LoadReplacementParameters::key, LoadReplacementParameters::json_user_function);
            register_json_user_function(UpdateGallery::key, UpdateGallery::json_user_function);

            // Render
            register_json_user_function(SetRenderFps::key, SetRenderFps::json_user_function);
            register_json_user_function(SetTexturesLazy::key, SetTexturesLazy::json_user_function);

            // Instances
            register_json_user_function(AddToInventory::key, AddToInventory::json_user_function);
            register_json_user_function(AddWeaponToInventory::key, AddWeaponToInventory::json_user_function);
            register_json_user_function(ConnectTrailer::key, ConnectTrailer::json_user_function);
            register_json_user_function(BurnIn::key, BurnIn::json_user_function);
            register_json_user_function(ClearParameters::key, ClearParameters::json_user_function);
            register_json_user_function(ClearRenderableInstance::key, ClearRenderableInstance::json_user_function);
            register_json_user_function(ClearSkybox::key, ClearSkybox::json_user_function);
            register_json_user_function(ConsoleLog::key, ConsoleLog::json_user_function);
            register_json_user_function(Controls::key, Controls::json_user_function);
            register_json_user_function(CreateAbsKeyBinding::key, CreateAbsKeyBinding::json_user_function);
            register_json_user_function(CreateAvatarControllerIdleBinding::key, CreateAvatarControllerIdleBinding::json_user_function);
            register_json_user_function(CreateAvatarControllerKeyBinding::key, CreateAvatarControllerKeyBinding::json_user_function);
            register_json_user_function(CreateCarControllerIdleBinding::key, CreateCarControllerIdleBinding::json_user_function);
            register_json_user_function(CreateCarControllerKeyBinding::key, CreateCarControllerKeyBinding::json_user_function);
            register_json_user_function(CreateCarController::key, CreateCarController::json_user_function);
            register_json_user_function(CreateAimAt::key, CreateAimAt::json_user_function);
            register_json_user_function(CreateCrash::key, CreateCrash::json_user_function);
            register_json_user_function(CreateDamageable::key, CreateDamageable::json_user_function);
            register_json_user_function(CreateDeltaEngine::key, CreateDeltaEngine::json_user_function);
            register_json_user_function(CreateDestinationReachedAi::key, CreateDestinationReachedAi::json_user_function);
            register_json_user_function(CreateEngine::key, CreateEngine::json_user_function);
            register_json_user_function(CreateGunKeyBinding::key, CreateGunKeyBinding::json_user_function);
            register_json_user_function(CreateGun::key, CreateGun::json_user_function);
            register_json_user_function(CreateMissileAi::key, CreateMissileAi::json_user_function);
            register_json_user_function(CreateHeliController::key, CreateHeliController::json_user_function);
            register_json_user_function(CreateHudOpponentTracker::key, CreateHudOpponentTracker::json_user_function);
            register_json_user_function(CreateHudOpponentZoom::key, CreateHudOpponentZoom::json_user_function);
            register_json_user_function(CreateHudTargetPointLogic::key, CreateHudTargetPointLogic::json_user_function);
            register_json_user_function(CreateAvatarAsAvatarController::key, CreateAvatarAsAvatarController::json_user_function);
            register_json_user_function(CreateAvatarAsCarController::key, CreateAvatarAsCarController::json_user_function);
            register_json_user_function(CreateKeepOffsetFromCamera::key, CreateKeepOffsetFromCamera::json_user_function);
            register_json_user_function(CreateKeepOffsetFromMovable::key, CreateKeepOffsetFromMovable::json_user_function);
            register_json_user_function(CreateLightOnlyShadow::key, CreateLightOnlyShadow::json_user_function);
            register_json_user_function(CreateLightWithoutShadow::key, CreateLightWithoutShadow::json_user_function);
            register_json_user_function(CreateLightWithShadow::key, CreateLightWithShadow::json_user_function);
            register_json_user_function(CreateMissileController::key, CreateMissileController::json_user_function);
            register_json_user_function(CreatePlaneAsCarController::key, CreatePlaneAsCarController::json_user_function);
            register_json_user_function(CreatePlaneControllerIdleBinding::key, CreatePlaneControllerIdleBinding::json_user_function);
            register_json_user_function(CreatePlaneControllerKeyBinding::key, CreatePlaneControllerKeyBinding::json_user_function);
            register_json_user_function(CreatePlaneController::key, CreatePlaneController::json_user_function);
            register_json_user_function(CreatePlayer::key, CreatePlayer::json_user_function);
            register_json_user_function(CreateRelativeTransformer::key, CreateRelativeTransformer::json_user_function);
            register_json_user_function(CreateCopyRotation::key, CreateCopyRotation::json_user_function);
            register_json_user_function(CreatePrintCameraNodeInfoKeyBinding::key, CreatePrintCameraNodeInfoKeyBinding::json_user_function);
            register_json_user_function(CreateRelKeyBinding::key, CreateRelKeyBinding::json_user_function);
            register_json_user_function(CreateRelKeyBindingTripod::key, CreateRelKeyBindingTripod::json_user_function);
            register_json_user_function(CreateRotor::key, CreateRotor::json_user_function);
            register_json_user_function(CreateSpawner::key, CreateSpawner::json_user_function);
            register_json_user_function(CreateTankController::key, CreateTankController::json_user_function);
            register_json_user_function(CreateTrailerNode::key, CreateTrailerNode::json_user_function);
            register_json_user_function(CreateVisualNodeStatus::key, CreateVisualNodeStatus::json_user_function);
            register_json_user_function(CreateVisualPlayerBulletCount::key, CreateVisualPlayerBulletCount::json_user_function);
            register_json_user_function(CreateVisualPlayerStatus::key, CreateVisualPlayerStatus::json_user_function);
            register_json_user_function(CreateWeaponCycleKeyBinding::key, CreateWeaponCycleKeyBinding::json_user_function);
            register_json_user_function(CreateWeaponCycle::key, CreateWeaponCycle::json_user_function);
            register_json_user_function(CreateWheel::key, CreateWheel::json_user_function);
            register_json_user_function(CreateWing::key, CreateWing::json_user_function);
            register_json_user_function(CreateYawPitchLookatNodes::key, CreateYawPitchLookatNodes::json_user_function);
            register_json_user_function(DefineWinnerConditionals::key, DefineWinnerConditionals::json_user_function);
            register_json_user_function(MoveNodeToBvh::key, MoveNodeToBvh::json_user_function);
            register_json_user_function(TryDeleteNode::key, TryDeleteNode::json_user_function);
            register_json_user_function(TryDeleteRootNode::key, TryDeleteRootNode::json_user_function);
            register_json_user_function(DeleteNode::key, DeleteNode::json_user_function);
            register_json_user_function(DeleteRootNodes::key, DeleteRootNodes::json_user_function);
            register_json_user_function(DeleteRootNode::key, DeleteRootNode::json_user_function);
            register_json_user_function(ExecuteInPhysicsThread::key, ExecuteInPhysicsThread::json_user_function);
            register_json_user_function(FillPixelRegionWithTexture::key, FillPixelRegionWithTexture::json_user_function);
            register_json_user_function(FollowNode::key, FollowNode::json_user_function);
            register_json_user_function(Minimap::key, Minimap::json_user_function);
            register_json_user_function(InstantiateGrass::key, InstantiateGrass::json_user_function);
            register_json_user_function(InvalidateAggregateRenderers::key, InvalidateAggregateRenderers::json_user_function);
            register_json_user_function(LoadPlayers::key, LoadPlayers::json_user_function);
            register_json_user_function(LookAtNode::key, LookAtNode::json_user_function);
            register_json_user_function(CreateDriveOrWalkAi::key, CreateDriveOrWalkAi::json_user_function);
            register_json_user_function(CreateVehicleFollowerAi::key, CreateVehicleFollowerAi::json_user_function);
            register_json_user_function(CreateOrthoCamera::key, CreateOrthoCamera::json_user_function);
            register_json_user_function(CreatePerspectiveCamera::key, CreatePerspectiveCamera::json_user_function);
            register_json_user_function(PlaybackTrack::key, PlaybackTrack::json_user_function);
            register_json_user_function(PlaybackWinnerTrack::key, PlaybackWinnerTrack::json_user_function);
            register_json_user_function(PlayerChangeAimingGun::key, PlayerChangeAimingGun::json_user_function);
            register_json_user_function(PlayerSetAimingGun::key, PlayerSetAimingGun::json_user_function);
            register_json_user_function(PlayerSetBehavior::key, PlayerSetBehavior::json_user_function);
            register_json_user_function(PlayerSetCanAim::key, PlayerSetCanAim::json_user_function);
            register_json_user_function(PlayerSetCanDrive::key, PlayerSetCanDrive::json_user_function);
            register_json_user_function(PlayerSetCanSelectBestWeapon::key, PlayerSetCanSelectBestWeapon::json_user_function);
            register_json_user_function(PlayerSetCanSelectOpponent::key, PlayerSetCanSelectOpponent::json_user_function);
            register_json_user_function(PlayerSetCanShoot::key, PlayerSetCanShoot::json_user_function);
            register_json_user_function(PlayerSetPlaybackWaypoints::key, PlayerSetPlaybackWaypoints::json_user_function);
            register_json_user_function(PlayerSetVehicleControlParameters::key, PlayerSetVehicleControlParameters::json_user_function);
            register_json_user_function(PlayerSetWaypoint::key, PlayerSetWaypoint::json_user_function);
            register_json_user_function(Preload::key, Preload::json_user_function);
            register_json_user_function(RecordTrackGpx::key, RecordTrackGpx::json_user_function);
            register_json_user_function(RecordTrack::key, RecordTrack::json_user_function);
            register_json_user_function(RegisterGravity::key, RegisterGravity::json_user_function);
            register_json_user_function(RegisterWind::key, RegisterWind::json_user_function);
            register_json_user_function(RootRenderableInstances::key, RootRenderableInstances::json_user_function);
            register_json_user_function(RespawnAllPlayers::key, RespawnAllPlayers::json_user_function);
            register_json_user_function(SaveToObjFile::key, SaveToObjFile::json_user_function);
            register_json_user_function(SceneToPixelRegion::key, SceneToPixelRegion::json_user_function);
            register_json_user_function(SceneToTexture::key, SceneToTexture::json_user_function);
            register_json_user_function(SetBackgroundColor::key, SetBackgroundColor::json_user_function);
            register_json_user_function(FitCanvasToRenderables::key, FitCanvasToRenderables::json_user_function);
            register_json_user_function(SetActorTask::key, SetActorTask::json_user_function);
            register_json_user_function(SetBevelBoxSurfaceNormal::key, SetBevelBoxSurfaceNormal::json_user_function);
            register_json_user_function(SetCapsuleSurfaceNormal::key, SetCapsuleSurfaceNormal::json_user_function);
            register_json_user_function(SetSlidingNormalModifier::key, SetSlidingNormalModifier::json_user_function);
            register_json_user_function(SetCameraCycle::key, SetCameraCycle::json_user_function);
            register_json_user_function(SetCamera::key, SetCamera::json_user_function);
            register_json_user_function(SetDesiredWeapon::key, SetDesiredWeapon::json_user_function);
            register_json_user_function(SetDirtmap::key, SetDirtmap::json_user_function);
            register_json_user_function(SetExternalsCreator::key, SetExternalsCreator::json_user_function);
            register_json_user_function(SetInventoryCapacity::key, SetInventoryCapacity::json_user_function);
            register_json_user_function(SetNodeBone::key, SetNodeBone::json_user_function);
            register_json_user_function(SetNodeRotation::key, SetNodeRotation::json_user_function);
            register_json_user_function(SetRaceIdentifierAndReloadHistory::key, SetRaceIdentifierAndReloadHistory::json_user_function);
            register_json_user_function(SetRigidBodyAlignToSurfaceRelaxation::key, SetRigidBodyAlignToSurfaceRelaxation::json_user_function);
            register_json_user_function(SetRigidBodyDoorDistance::key, SetRigidBodyDoorDistance::json_user_function);
            register_json_user_function(SetRigidBodyGrindPoint::key, SetRigidBodyGrindPoint::json_user_function);
            register_json_user_function(SetRigidBodyRevertSurfacePowerThreshold::key, SetRigidBodyRevertSurfacePowerThreshold::json_user_function);
            register_json_user_function(SetRigidBodyTarget::key, SetRigidBodyTarget::json_user_function);
            register_json_user_function(SetSkybox::key, SetSkybox::json_user_function);
            register_json_user_function(SetObjective::key, SetObjective::json_user_function);
            register_json_user_function(ResetSupplyDepotCooldowns::key, ResetSupplyDepotCooldowns::json_user_function);
            register_json_user_function(SetSoftLight::key, SetSoftLight::json_user_function);
            register_json_user_function(SetSpawnPoints::key, SetSpawnPoints::json_user_function);
            register_json_user_function(SetWayPoints::key, SetWayPoints::json_user_function);
            register_json_user_function(SpawnerSetNodes::key, SpawnerSetNodes::json_user_function);
            register_json_user_function(SpawnerSetPlayer::key, SpawnerSetPlayer::json_user_function);
            register_json_user_function(SpawnerSetRespawnCooldownTime::key, SpawnerSetRespawnCooldownTime::json_user_function);
            register_json_user_function(StartRace::key, StartRace::json_user_function);
            register_json_user_function(TeamSetWaypoint::key, TeamSetWaypoint::json_user_function);
            register_json_user_function(UiExhibit::key, UiExhibit::json_user_function);
            register_json_user_function(VisualNodeStatus3rd::key, VisualNodeStatus3rd::json_user_function);

            // Resources
            register_json_user_function(AddAudio::key, AddAudio::json_user_function);
            register_json_user_function(AddAudioSequence::key, AddAudioSequence::json_user_function);
            register_json_user_function(AddBlendMapTexture::key, AddBlendMapTexture::json_user_function);
            register_json_user_function(AddBvhResource::key, AddBvhResource::json_user_function);
            register_json_user_function(AddCompanionRenderable::key, AddCompanionRenderable::json_user_function);
            register_json_user_function(AddFoliageResource::key, AddFoliageResource::json_user_function);
            register_json_user_function(AddTextureAtlas::key, AddTextureAtlas::json_user_function);
            register_json_user_function(AnimatableBillboards::key, AnimatableBillboards::json_user_function);
            register_json_user_function(AnimatedBillboards::key, AnimatedBillboards::json_user_function);
            register_json_user_function(AnimatableTrails::key, AnimatableTrails::json_user_function);
            register_json_user_function(AnimatedTrails::key, AnimatedTrails::json_user_function);
            register_json_user_function(CleanupMesh::key, CleanupMesh::json_user_function);
            register_json_user_function(CreateBinaryXResource::key, CreateBinaryXResource::json_user_function);
            register_json_user_function(CreateBlendingXResource::key, CreateBlendingXResource::json_user_function);
            register_json_user_function(ConvexDecomposeTerrain::key, ConvexDecomposeTerrain::json_user_function);
            register_json_user_function(DeleteMesh::key, DeleteMesh::json_user_function);
            register_json_user_function(Downsample::key, Downsample::json_user_function);
            register_json_user_function(GenRay::key, GenRay::json_user_function);
            register_json_user_function(GenTriangleRays::key, GenTriangleRays::json_user_function);
            register_json_user_function(GenGrindLines::key, GenGrindLines::json_user_function);
            register_json_user_function(GenContourEdges::key, GenContourEdges::json_user_function);
            register_json_user_function(GenInstances::key, GenInstances::json_user_function);
            register_json_user_function(ImportBoneWeights::key, ImportBoneWeights::json_user_function);
            register_json_user_function(LoadOsmResource::key, LoadOsmResource::json_user_function);
            register_json_user_function(ObjResource::key, ObjResource::json_user_function);
            register_json_user_function(Repeat::key, Repeat::json_user_function);
            register_json_user_function(SaveTextureArrayPng::key, SaveTextureArrayPng::json_user_function);
            register_json_user_function(SaveTexturePng::key, SaveTexturePng::json_user_function);
            register_json_user_function(CreateSquareResource::key, CreateSquareResource::json_user_function);
            register_json_user_function(CreateGridResource::key, CreateGridResource::json_user_function);
            register_json_user_function(MergeBlendedMaterials::key, MergeBlendedMaterials::json_user_function);
            register_json_user_function(MergeMeshes::key, MergeMeshes::json_user_function);
            register_json_user_function(ModifyPhysicsMaterialTags::key, ModifyPhysicsMaterialTags::json_user_function);
            register_json_user_function(ModifyRenderingMaterial::key, ModifyRenderingMaterial::json_user_function);
            register_json_user_function(PrintResource::key, PrintResource::json_user_function);
            register_json_user_function(CreateTapButton::key, CreateTapButton::json_user_function);
            register_json_user_function(SetAnimatedDynamicLightProperties::key, SetAnimatedDynamicLightProperties::json_user_function);
            register_json_user_function(SetBounds::key, SetBounds::json_user_function);
            register_json_user_function(SetConstantDynamicLightProperties::key, SetConstantDynamicLightProperties::json_user_function);
            register_json_user_function(SetBulletProperties::key, SetBulletProperties::json_user_function);
            register_json_user_function(SetSurfaceContactInfo::key, SetSurfaceContactInfo::json_user_function);
            register_json_user_function(SmoothenEdges::key, SmoothenEdges::json_user_function);
            register_json_user_function(ShadeAuto::key, ShadeAuto::json_user_function);
        }
    } add_funcs;
}

LoadScene::~LoadScene() = default;

void LoadScene::operator () () {
    macro_file_executor_(macro_line_executor_);
}
