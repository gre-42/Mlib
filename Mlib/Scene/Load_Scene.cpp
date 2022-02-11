#include "Load_Scene.hpp"
#include <Mlib/Macro_Line_Executor.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Instances/Add_Node_Not_Allowed_To_Be_Unregistered.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Instances/Add_Weapon_To_Inventory.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Instances/Append_Externals_Deleter.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Instances/Burn_In.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Instances/Clear_Absolute_Observer.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Instances/Clear_Nodes_Not_Allowed_To_Be_Unregistered.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Instances/Clear_Parameters.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Instances/Clear_Renderable_Instance.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Instances/Console_Log.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Instances/Controls.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Instances/Countdown.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Instances/Create_Abs_Key_Binding.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Instances/Create_Avatar_Controller_Idle_Binding.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Instances/Create_Avatar_Controller_Key_Binding.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Instances/Create_Camera_Key_Binding.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Instances/Create_Car_Controller.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Instances/Create_Car_Controller_Idle_Binding.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Instances/Create_Car_Controller_Key_Binding.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Instances/Create_Check_Points.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Instances/Create_Child_Node.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Instances/Create_Crash.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Instances/Create_Damageable.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Instances/Create_Driver_Key_Binding.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Instances/Create_Engine.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Instances/Create_Externals.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Instances/Create_Gun.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Instances/Create_Gun_Key_Binding.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Instances/Create_Heli_Controller.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Instances/Create_Human_As_Avatar_Controller.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Instances/Create_Human_As_Car_Controller.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Instances/Create_Light.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Instances/Create_Parameter_Setter_Logic.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Instances/Create_Player.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Instances/Create_Rel_Key_Binding.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Instances/Create_Relative_Transformer.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Instances/Create_Rigid_Cuboid.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Instances/Create_Rigid_Disk.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Instances/Create_Rotor.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Instances/Create_Scene_Selector_Logic.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Instances/Create_Tab_Menu_Logic.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Instances/Create_Tank_Controller.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Instances/Create_Visual_Global_Log.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Instances/Create_Visual_Node_Status.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Instances/Create_Visual_Player_Status.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Instances/Create_Weapon_Inventory.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Instances/Create_Weapon_Inventory_Key_Binding.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Instances/Create_Wheel.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Instances/Create_Yaw_Pitch_Lookat_Nodes.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Instances/Define_Winner_Conditionals.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Instances/Delete_Root_Node.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Instances/Delete_Root_Nodes.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Instances/Delete_Scheduled_Advance_Times.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Instances/Equip_Weapon.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Instances/Execute_In_Physics_Thread.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Instances/Fill_Pixel_Region_With_Texture.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Instances/Focused_Text.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Instances/Follow_Node.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Instances/Hud_Image.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Instances/Keep_Offset.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Instances/Load_Players.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Instances/Look_At_Node.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Instances/Ortho_Camera.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Instances/Pause_On_Lose_Focus.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Instances/Perspective_Camera.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Instances/Playback_Track.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Instances/Playback_Winner_Track.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Instances/Player_Set_Aiming_Gun.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Instances/Player_Set_Can_Aim.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Instances/Player_Set_Can_Drive.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Instances/Player_Set_Can_Shoot.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Instances/Player_Set_Node.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Instances/Player_Set_Surface_Power.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Instances/Player_Set_Waypoint.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Instances/Player_Set_Waypoints.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Instances/Players_Stats.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Instances/Pod_Bot_Set_Game_Mode.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Instances/Pod_Bot_Set_Waypoints.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Instances/Record_Track.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Instances/Record_Track_Gpx.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Instances/Register_Geographic_Mapping.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Instances/Remove_Node_Not_Allowed_To_Be_Unregistered.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Instances/Renderable_Instance.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Instances/Respawn_All_Players.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Instances/Root_Node_Instance.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Instances/Scene_To_Pixel_Region.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Instances/Scene_To_Texture.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Instances/Set_Avatar_Style_Updater.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Instances/Set_Camera.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Instances/Set_Camera_Cycle.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Instances/Set_Dirtmap.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Instances/Set_Externals_Creator.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Instances/Set_Preferred_Car_Spawner.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Instances/Set_Renderable_Style.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Instances/Set_Rigid_Body_Target.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Instances/Set_Skybox.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Instances/Set_Soft_Light.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Instances/Set_Spawn_Points.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Instances/Set_Vip.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Instances/Team_Set_Waypoint.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Instances/Ui_Background.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Instances/Visual_Node_Status_3rd.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Resources/Add_Audio.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Resources/Add_Blend_Map_Texture.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Resources/Add_Bvh_Resource.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Resources/Add_Companion_Renderable.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Resources/Add_Texture_Atlas.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Resources/Add_Texture_Descriptor.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Resources/Append_Focuses.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Resources/Create_Binary_X_Resource.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Resources/Create_Blending_X_Resource.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Resources/Create_Scene.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Resources/Create_Square_Resource.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Resources/Downsample.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Resources/Gen_Ray.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Resources/Gen_Triangle_Rays.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Resources/Import_Bone_Weights.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Resources/Load_Osm_Resource.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Resources/Load_Osm_Resource.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Resources/Obj_Resource.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Resources/Repeat.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Resources/Save_Texture_Atlas_Png.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Resources/Set_Focuses.hpp>
#include <Mlib/Scene/User_Function_Args.hpp>

using namespace Mlib;

LoadScene::LoadScene() {
    user_functions_.push_back(AddWeaponToInventory::user_function);
    user_functions_.push_back(AppendExternalsDeleter::user_function);
    user_functions_.push_back(CreateAbsKeyBinding::user_function);
    user_functions_.push_back(CreateAvatarControllerIdleBinding::user_function);
    user_functions_.push_back(CreateAvatarControllerKeyBinding::user_function);
    user_functions_.push_back(CreateCameraKeyBinding::user_function);
    user_functions_.push_back(CreateCarControllerIdleBinding::user_function);
    user_functions_.push_back(CreateCarControllerKeyBinding::user_function);
    user_functions_.push_back(CreateCarController::user_function);
    user_functions_.push_back(CreateCheckPoints::user_function);
    user_functions_.push_back(CreateChildNode::user_function);
    user_functions_.push_back(CreateDriverKeyBinding::user_function);
    user_functions_.push_back(CreateExternals::user_function);
    user_functions_.push_back(CreateGunKeyBinding::user_function);
    user_functions_.push_back(CreateHeliController::user_function);
    user_functions_.push_back(CreateHumanAsAvatarController::user_function);
    user_functions_.push_back(CreateHumanAsCarController::user_function);
    user_functions_.push_back(CreatePlayer::user_function);
    user_functions_.push_back(CreateRelKeyBinding::user_function);
    user_functions_.push_back(CreateRigidCuboid::user_function);
    user_functions_.push_back(CreateRigidDisk::user_function);
    user_functions_.push_back(CreateRotor::user_function);
    user_functions_.push_back(CreateTabMenuLogic::user_function);
    user_functions_.push_back(CreateTankController::user_function);
    user_functions_.push_back(CreateVisualNodeStatus::user_function);
    user_functions_.push_back(CreateVisualPlayerStatus::user_function);
    user_functions_.push_back(CreateWeaponInventory::user_function);
    user_functions_.push_back(CreateWeaponInventoryKeyBinding::user_function);
    user_functions_.push_back(CreateWheel::user_function);
    user_functions_.push_back(CreateYawPitchLookatNodes::user_function);
    user_functions_.push_back(EquipWeapon::user_function);
    user_functions_.push_back(LoadPlayers::user_function);
    user_functions_.push_back(PlayerSetAimingGun::user_function);
    user_functions_.push_back(PlayerSetCanAim::user_function);
    user_functions_.push_back(PlayerSetCanDrive::user_function);
    user_functions_.push_back(PlayerSetCanShoot::user_function);
    user_functions_.push_back(PlayerSetNode::user_function);
    user_functions_.push_back(PlayerSetSurfacePower::user_function);
    user_functions_.push_back(PlayerSetWaypoints::user_function);
    user_functions_.push_back(PlayerSetWaypoint::user_function);
    user_functions_.push_back(PodBotSetGameMode::user_function);
    user_functions_.push_back(PodBotSetWaypoints::user_function);
    user_functions_.push_back(SetExternalsCreator::user_function);
    user_functions_.push_back(SetPreferredCarSpawner::user_function);
    user_functions_.push_back(SetRigidBodyTarget::user_function);
    user_functions_.push_back(SetSpawnPoints::user_function);
    user_functions_.push_back(TeamSetWaypoint::user_function);
    user_functions_.push_back(AddNodeNotAllowedToBeUnregistered::user_function);
    user_functions_.push_back(ClearAbsoluteObserver::user_function);
    user_functions_.push_back(ClearNodesNotAllowedToBeUnregistered::user_function);
    user_functions_.push_back(Controls::user_function);
    user_functions_.push_back(Countdown::user_function);
    user_functions_.push_back(CreateCrash::user_function);
    user_functions_.push_back(CreateEngine::user_function);
    user_functions_.push_back(CreateVisualGlobalLog::user_function);
    user_functions_.push_back(DefineWinnerConditionals::user_function);
    user_functions_.push_back(DeleteRootNodes::user_function);
    user_functions_.push_back(FillPixelRegionWithTexture::user_function);
    user_functions_.push_back(FollowNode::user_function);
    user_functions_.push_back(CreateGun::user_function);
    user_functions_.push_back(HudImage::user_function);
    user_functions_.push_back(LookAtNode::user_function);
    user_functions_.push_back(OrthoCamera::user_function);
    user_functions_.push_back(CreateParameterSetterLogic::user_function);
    user_functions_.push_back(PlayersStats::user_function);
    user_functions_.push_back(RecordTrackGpx::user_function);
    user_functions_.push_back(RenderableInstance::user_function);
    user_functions_.push_back(RespawnAllPlayers::user_function);
    user_functions_.push_back(RootNodeInstance::user_function);
    user_functions_.push_back(CreateSceneSelectorLogic::user_function);
    user_functions_.push_back(SetCameraCycle::user_function);
    user_functions_.push_back(SetDirtmap::user_function);
    user_functions_.push_back(SetRenderableStyle::user_function);
    user_functions_.push_back(SetSkybox::user_function);
    user_functions_.push_back(BurnIn::user_function);
    user_functions_.push_back(ClearParameters::user_function);
    user_functions_.push_back(ClearRenderableInstance::user_function);
    user_functions_.push_back(ConsoleLog::user_function);
    user_functions_.push_back(CreateDamageable::user_function);
    user_functions_.push_back(CreateLight::user_function);
    user_functions_.push_back(DeleteRootNode::user_function);
    user_functions_.push_back(DeleteScheduledAdvanceTimes::user_function);
    user_functions_.push_back(ExecuteInPhysicsThread::user_function);
    user_functions_.push_back(FocusedText::user_function);
    user_functions_.push_back(KeepOffset::user_function);
    user_functions_.push_back(PauseOnLoseFocus::user_function);
    user_functions_.push_back(PerspectiveCamera::user_function);
    user_functions_.push_back(PlaybackTrack::user_function);
    user_functions_.push_back(PlaybackWinnerTrack::user_function);
    user_functions_.push_back(RecordTrack::user_function);
    user_functions_.push_back(RegisterGeographicMapping::user_function);
    user_functions_.push_back(CreateRelativeTransformer::user_function);
    user_functions_.push_back(RemoveNodeNotAllowedToBeUnregistered::user_function);
    user_functions_.push_back(SceneToPixelRegion::user_function);
    user_functions_.push_back(SceneToTexture::user_function);
    user_functions_.push_back(SetAvatarStyleUpdater::user_function);
    user_functions_.push_back(SetCamera::user_function);
    user_functions_.push_back(SetSoftLight::user_function);
    user_functions_.push_back(SetVip::user_function);
    user_functions_.push_back(UiBackground::user_function);
    user_functions_.push_back(VisualNodeStatus3rd::user_function);

    user_functions_.push_back(LoadOsmResource::user_function);
    user_functions_.push_back(AddAudio::user_function);
    user_functions_.push_back(AddBlendMapTexture::user_function);
    user_functions_.push_back(AddBvhResource::user_function);
    user_functions_.push_back(AddCompanionRenderable::user_function);
    user_functions_.push_back(AddTextureAtlas::user_function);
    user_functions_.push_back(AddTextureDescriptor::user_function);
    user_functions_.push_back(AppendFocuses::user_function);
    user_functions_.push_back(CreateBinaryXResource::user_function);
    user_functions_.push_back(CreateBlendingXResource::user_function);
    user_functions_.push_back(CreateScene::user_function);
    user_functions_.push_back(Downsample::user_function);
    user_functions_.push_back(GenRay::user_function);
    user_functions_.push_back(GenTriangleRays::user_function);
    user_functions_.push_back(ImportBoneWeights::user_function);
    user_functions_.push_back(LoadOsmResource::user_function);
    user_functions_.push_back(LoadOsmResource::user_function);
    user_functions_.push_back(ObjResource::user_function);
    user_functions_.push_back(Repeat::user_function);
    user_functions_.push_back(SaveTextureAtlasPng::user_function);
    user_functions_.push_back(SetFocuses::user_function);
    user_functions_.push_back(CreateSquareResource::user_function);
}

LoadScene::~LoadScene()
{}

void LoadScene::operator()(
    const std::string& working_directory,
    const std::string& script_filename,
    std::string& next_scene_filename,
    SubstitutionMap& external_substitutions,
    size_t& num_renderings,
    bool verbose,
    RegexSubstitutionCache& rsc,
    SceneNodeResources& scene_node_resources,
    SceneConfig& scene_config,
    ButtonStates& button_states,
    CursorStates& cursor_states,
    CursorStates& scroll_wheel_states,
    UiFocus& ui_focus,
    GLFWwindow* window,
    std::map<std::string, std::shared_ptr<RenderableScene>>& renderable_scenes)
{
    MacroLineExecutor::UserFunction user_function = [&](
        const std::string& context,
        const std::function<FPath(const std::string&)>& fpath,
        const MacroLineExecutor& macro_line_executor,
        const std::string& line,
        SubstitutionMap* local_substitutions) -> bool
    {
        auto renderable_scene = [&]() -> RenderableScene& {
            auto cit = renderable_scenes.find(context);
            if (cit == renderable_scenes.end()) {
                throw std::runtime_error("Could not find renderable scene with name \"" + context + '"');
            }
            return *cit->second;
        };
        LoadSceneUserFunctionArgs args{
            .line = line,
            .renderable_scene = renderable_scene,
            .fpath = fpath,
            .macro_line_executor = macro_line_executor,
            .external_substitutions = external_substitutions,
            .local_substitutions = local_substitutions,
            .rsc = rsc,
            .scene_node_resources = scene_node_resources,
            .scene_config = scene_config,
            .button_states = button_states,
            .cursor_states = cursor_states,
            .scroll_wheel_states = scroll_wheel_states,
            .ui_focus = ui_focus,
            .window = window,
            .num_renderings = num_renderings,
            .script_filename = script_filename,
            .next_scene_filename = next_scene_filename,
            .renderable_scenes = renderable_scenes};
        for (const auto& f : user_functions_) {
            if (f(args))
            {
                return true;
            }
        }
        return false;
    };
    MacroLineExecutor lp2{
        macro_file_executor_,
        script_filename,
        working_directory,
        user_function,
        "no_scene_specified",
        external_substitutions,
        verbose};
    macro_file_executor_(lp2, rsc);
}
