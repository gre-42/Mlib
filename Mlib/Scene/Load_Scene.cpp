#include "Load_Scene.hpp"
#include <Mlib/Macro_Executor/Json_Macro_Arguments.hpp>
#include <Mlib/Macro_Executor/Macro_Line_Executor.hpp>
#include <Mlib/Scene/Json_User_Function_Args.hpp>
#include <Mlib/Scene/Json_User_Function_Args.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Constant_Parameter.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Containers/Add_To_Gallery.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Containers/Create_Scene.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Containers/Load_Macro_Manifests.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Containers/Load_Replacement_Parameters.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Containers/Update_Gallery.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Create_Additive_Screen_Constraint.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Create_Constant_Screen_Constraint.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Create_Fractional_Screen_Constraint.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Create_Tap_Button.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Echo.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Instances/Cameras/Create_Ortho_Camera.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Instances/Cameras/Create_Perspective_Camera.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Instances/Cameras/Set_Camera.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Instances/Cameras/Set_Camera_Cycle.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Instances/Controllers/Create_Car_Controller.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Instances/Controllers/Create_Heli_Controller.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Instances/Controllers/Create_Human_As_Avatar_Controller.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Instances/Controllers/Create_Human_As_Car_Controller.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Instances/Controllers/Create_Plane_As_Car_Controller.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Instances/Controllers/Create_Plane_Controller.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Instances/Controllers/Create_Tank_Controller.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Instances/Create_Check_Points.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Instances/Define_Winner_Conditionals.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Instances/Delete_Scheduled_Advance_Times.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Instances/Execute_In_Physics_Thread.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Instances/Hud/Console_Log.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Instances/Hud/Create_Visual_Global_Log.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Instances/Hud/Create_Visual_Node_Status.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Instances/Hud/Create_Visual_Player_Bullet_Count.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Instances/Hud/Create_Visual_Player_Status.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Instances/Hud/Fill_Pixel_Region_With_Texture.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Instances/Hud/Focused_Text.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Instances/Hud/Hud_Image.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Instances/Hud/Players_Stats.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Instances/Hud/Scene_To_Percentage_Region.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Instances/Hud/Scene_To_Pixel_Region.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Instances/Hud/Scene_To_Texture.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Instances/Hud/Visual_Node_Status_3rd.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Instances/Invalidate_Aggregate_Renderers.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Instances/Inventory/Set_Desired_Weapon.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Instances/Inventory/Set_Inventory_Capacity.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Instances/Key_Bindings/Create_Abs_Idle_Key_Binding.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Instances/Key_Bindings/Create_Abs_Key_Binding.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Instances/Key_Bindings/Create_Avatar_Controller_Idle_Binding.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Instances/Key_Bindings/Create_Avatar_Controller_Key_Binding.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Instances/Key_Bindings/Create_Camera_Key_Binding.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Instances/Key_Bindings/Create_Car_Controller_Idle_Binding.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Instances/Key_Bindings/Create_Car_Controller_Key_Binding.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Instances/Key_Bindings/Create_Driver_Key_Binding.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Instances/Key_Bindings/Create_Gun_Key_Binding.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Instances/Key_Bindings/Create_Plane_Controller_Idle_Binding.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Instances/Key_Bindings/Create_Plane_Controller_Key_Binding.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Instances/Key_Bindings/Create_Rel_Key_Binding.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Instances/Key_Bindings/Create_Weapon_Cycle_Key_Binding.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Instances/Key_Bindings/Load_Key_Configurations.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Instances/Lights/Create_Light_Only_Shadow.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Instances/Lights/Create_Light_With_Shadow.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Instances/Lights/Create_Light_Without_Shadow.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Instances/Lights/Set_Soft_Light.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Instances/Nodes/Add_Color_Style.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Instances/Nodes/Add_Node_Not_Allowed_To_Be_Unregistered.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Instances/Nodes/Append_Externals_Deleter.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Instances/Nodes/Clear_Nodes_Not_Allowed_To_Be_Unregistered.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Instances/Nodes/Clear_Renderable_Instance.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Instances/Nodes/Create_Child_Node.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Instances/Nodes/Create_Copy_Rotation.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Instances/Nodes/Create_Externals.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Instances/Nodes/Create_Keep_Offset_From_Camera.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Instances/Nodes/Create_Keep_Offset_From_Movable.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Instances/Nodes/Create_Relative_Transformer.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Instances/Nodes/Delete_Node.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Instances/Nodes/Delete_Root_Node.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Instances/Nodes/Delete_Root_Nodes.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Instances/Nodes/Look_At_Node.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Instances/Nodes/Set_Animation_State.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Instances/Nodes/Set_Avatar_Style_Updater.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Instances/Nodes/Set_Node_Bone.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Instances/Nodes/Set_Node_Hider.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Instances/Nodes/Set_Node_Rotation.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Instances/Nodes/Try_Delete_Node.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Instances/Nodes/Try_Delete_Root_Node.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Instances/Players/Create_Player.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Instances/Players/Load_Players.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Instances/Players/Player_Set_Aiming_Gun.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Instances/Players/Player_Set_Can_Aim.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Instances/Players/Player_Set_Can_Drive.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Instances/Players/Player_Set_Can_Select_Best_Weapon.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Instances/Players/Player_Set_Can_Shoot.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Instances/Players/Player_Set_Node.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Instances/Players/Player_Set_Pathfinding_Waypoints.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Instances/Players/Player_Set_Playback_Waypoints.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Instances/Players/Player_Set_Vehicle_Control_Parameters.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Instances/Players/Player_Set_Waypoint.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Instances/Players/Respawn_All_Players.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Instances/Players/Set_Preferred_Car_Spawner.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Instances/Players/Set_Race_Identifier_And_Reload_History.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Instances/Players/Set_Spawn_Points.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Instances/Players/Set_Vip.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Instances/Players/Start_Race.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Instances/Players/Team_Set_Waypoint.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Instances/Preload.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Instances/Remove_Node_Not_Allowed_To_Be_Unregistered.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Instances/Render/Renderable_Instance.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Instances/Render/Set_Background_Color.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Instances/Render/Set_Dirtmap.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Instances/Render/Set_Skybox.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Instances/Reset_Supply_Depot_Cooldowns.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Instances/Root_Node_Instance.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Instances/Set_Externals_Creator.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Instances/Set_Objective.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Instances/Spawners/Create_Spawner.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Instances/Spawners/Spawner_Set_Player.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Instances/Track/Playback_Track.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Instances/Track/Playback_Winner_Track.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Instances/Track/Record_Track.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Instances/Track/Record_Track_Gpx.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Instances/Track/Register_Geographic_Mapping.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Instances/Ui/Clear_Parameters.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Instances/Ui/Controls.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Instances/Ui/Countdown.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Instances/Ui/Create_Parameter_Setter_Logic.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Instances/Ui/Create_Scene_Selector_Logic.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Instances/Ui/Create_Tab_Menu_Logic.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Instances/Ui/Ui_Background.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Instances/Ui/Ui_Exhibit.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Instances/Vehicles/Add_To_Inventory.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Instances/Vehicles/Add_Weapon_To_Cycle.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Instances/Vehicles/Burn_In.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Instances/Vehicles/Clear_Absolute_Observer_And_Notify_Destroyed.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Instances/Vehicles/Create_Crash.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Instances/Vehicles/Create_Damageable.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Instances/Vehicles/Create_Engine.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Instances/Vehicles/Create_Gun.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Instances/Vehicles/Create_Rigid_Cuboid.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Instances/Vehicles/Create_Rigid_Disk.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Instances/Vehicles/Create_Rotor.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Instances/Vehicles/Create_Weapon_Cycle.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Instances/Vehicles/Create_Wheel.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Instances/Vehicles/Create_Wing.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Instances/Vehicles/Create_Yaw_Pitch_Lookat_Nodes.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Instances/Vehicles/Follow_Node.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Instances/Vehicles/Set_Jump_Strength.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Instances/Vehicles/Set_RigidBody_Grind_Point.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Instances/Vehicles/Set_Rigid_Body_Align_To_Surface_Relaxation.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Instances/Vehicles/Set_Rigid_Body_Revert_Surface_Power_Threshold.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Instances/Vehicles/Set_Rigid_Body_Target.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Instances/Vehicles/Set_Skater_Style_Updater.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Instances/Vehicles/Ypln_Update_Bullet_Properties.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Instances/With_Delete_Node_Mutex.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Main/Clear_Selection_Ids.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Main/Reload_Scene.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Resources/Add_Audio.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Resources/Add_Audio_Sequence.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Resources/Add_Blend_Map_Texture.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Resources/Add_Bvh_Resource.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Resources/Add_Companion_Renderable.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Resources/Add_Cubemap.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Resources/Add_Texture_Atlas.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Resources/Add_Texture_Descriptor.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Resources/Append_Focuses.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Resources/Create_Binary_X_Resource.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Resources/Create_Blending_X_Resource.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Resources/Create_Grid_Resource.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Resources/Create_Square_Resource.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Resources/Downsample.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Resources/Gen_Compound_Resource.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Resources/Gen_Contour_Edges.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Resources/Gen_Grind_Lines.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Resources/Gen_Instances.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Resources/Gen_Ray.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Resources/Gen_Triangle_Rays.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Resources/Import_Bone_Weights.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Resources/Load_Osm_Resource.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Resources/Load_Osm_Resource.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Resources/Modify_Physics_Material_Tags.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Resources/Obj_Resource.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Resources/Print_Resource.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Resources/Repeat.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Resources/Save_Texture_Atlas_Png.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Resources/Set_Focuses.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Resources/Set_Surface_Contact_Info.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Sleep.hpp>
#include <Mlib/Scene/Renderable_Scene.hpp>
#include <Mlib/Scene/Renderable_Scenes.hpp>

using namespace Mlib;

static DECLARE_REGEX(func_name_re, "^\\s*(\\w+)\\s*([\\s\\S]*)$");

void LoadScene::register_json_user_function(const std::string& key, LoadSceneJsonUserFunction function) {
    if (!json_user_functions_.try_emplace(key, function).second) {
        THROW_OR_ABORT("Multiple functions with name \"" + key + "\" exist");
    }
}

LoadScene::LoadScene() {
    // Containers
    register_json_user_function(AddToGallery::key, AddToGallery::json_user_function);
    register_json_user_function(CreateScene::key, CreateScene::json_user_function);
    register_json_user_function(UpdateGallery::key, UpdateGallery::json_user_function);
    register_json_user_function(LoadMacroManifests::key, LoadMacroManifests::json_user_function);
    register_json_user_function(LoadReplacementParameters::key, LoadReplacementParameters::json_user_function);

    // Instances
    register_json_user_function(AddColorStyle::key, AddColorStyle::json_user_function);
    register_json_user_function(AddNodeNotAllowedToBeUnregistered::key, AddNodeNotAllowedToBeUnregistered::json_user_function);
    register_json_user_function(AddToInventory::key, AddToInventory::json_user_function);
    register_json_user_function(AddWeaponToInventory::key, AddWeaponToInventory::json_user_function);
    register_json_user_function(AppendExternalsDeleter::key, AppendExternalsDeleter::json_user_function);
    register_json_user_function(BurnIn::key, BurnIn::json_user_function);
    register_json_user_function(ClearAbsoluteObserverAndNotifyDestroyed::key, ClearAbsoluteObserverAndNotifyDestroyed::json_user_function);
    register_json_user_function(ClearNodesNotAllowedToBeUnregistered::key, ClearNodesNotAllowedToBeUnregistered::json_user_function);
    register_json_user_function(ClearParameters::key, ClearParameters::json_user_function);
    register_json_user_function(ClearRenderableInstance::key, ClearRenderableInstance::json_user_function);
    register_json_user_function(ConsoleLog::key, ConsoleLog::json_user_function);
    register_json_user_function(Controls::key, Controls::json_user_function);
    register_json_user_function(Countdown::key, Countdown::json_user_function);
    register_json_user_function(CreateAbsKeyBinding::key, CreateAbsKeyBinding::json_user_function);
    register_json_user_function(CreateAvatarControllerIdleBinding::key, CreateAvatarControllerIdleBinding::json_user_function);
    register_json_user_function(CreateAvatarControllerKeyBinding::key, CreateAvatarControllerKeyBinding::json_user_function);
    register_json_user_function(CreateCameraKeyBinding::key, CreateCameraKeyBinding::json_user_function);
    register_json_user_function(CreateCarControllerIdleBinding::key, CreateCarControllerIdleBinding::json_user_function);
    register_json_user_function(CreateCarControllerKeyBinding::key, CreateCarControllerKeyBinding::json_user_function);
    register_json_user_function(CreateCarController::key, CreateCarController::json_user_function);
    register_json_user_function(CreateCheckPoints::key, CreateCheckPoints::json_user_function);
    register_json_user_function(CreateChildNode::key, CreateChildNode::json_user_function);
    register_json_user_function(CreateCrash::key, CreateCrash::json_user_function);
    register_json_user_function(CreateDamageable::key, CreateDamageable::json_user_function);
    register_json_user_function(CreateDriverKeyBinding::key, CreateDriverKeyBinding::json_user_function);
    register_json_user_function(CreateEngine::key, CreateEngine::json_user_function);
    register_json_user_function(CreateExternals::key, CreateExternals::json_user_function);
    register_json_user_function(CreateGunKeyBinding::key, CreateGunKeyBinding::json_user_function);
    register_json_user_function(CreateGun::key, CreateGun::json_user_function);
    register_json_user_function(CreateHeliController::key, CreateHeliController::json_user_function);
    register_json_user_function(CreateHumanAsAvatarController::key, CreateHumanAsAvatarController::json_user_function);
    register_json_user_function(CreateHumanAsCarController::key, CreateHumanAsCarController::json_user_function);
    register_json_user_function(CreateKeepOffsetFromCamera::key, CreateKeepOffsetFromCamera::json_user_function);
    register_json_user_function(CreateKeepOffsetFromMovable::key, CreateKeepOffsetFromMovable::json_user_function);
    register_json_user_function(CreateLightOnlyShadow::key, CreateLightOnlyShadow::json_user_function);
    register_json_user_function(CreateLightWithoutShadow::key, CreateLightWithoutShadow::json_user_function);
    register_json_user_function(CreateLightWithShadow::key, CreateLightWithShadow::json_user_function);
    register_json_user_function(CreateParameterSetterLogic::key, CreateParameterSetterLogic::json_user_function);
    register_json_user_function(CreatePlaneAsCarController::key, CreatePlaneAsCarController::json_user_function);
    register_json_user_function(CreatePlaneControllerIdleBinding::key, CreatePlaneControllerIdleBinding::json_user_function);
    register_json_user_function(CreatePlaneControllerKeyBinding::key, CreatePlaneControllerKeyBinding::json_user_function);
    register_json_user_function(CreatePlaneController::key, CreatePlaneController::json_user_function);
    register_json_user_function(CreatePlayer::key, CreatePlayer::json_user_function);
    register_json_user_function(CreateRelativeTransformer::key, CreateRelativeTransformer::json_user_function);
    register_json_user_function(CreateCopyRotation::key, CreateCopyRotation::json_user_function);
    register_json_user_function(CreateRelKeyBinding::key, CreateRelKeyBinding::json_user_function);
    register_json_user_function(CreateRigidCuboid::key, CreateRigidCuboid::json_user_function);
    register_json_user_function(CreateRigidDisk::key, CreateRigidDisk::json_user_function);
    register_json_user_function(CreateRotor::key, CreateRotor::json_user_function);
    register_json_user_function(CreateSceneSelectorLogic::key, CreateSceneSelectorLogic::json_user_function);
    register_json_user_function(CreateSpawner::key, CreateSpawner::json_user_function);
    register_json_user_function(CreateTabMenuLogic::key, CreateTabMenuLogic::json_user_function);
    register_json_user_function(CreateTankController::key, CreateTankController::json_user_function);
    register_json_user_function(CreateVisualGlobalLog::key, CreateVisualGlobalLog::json_user_function);
    register_json_user_function(CreateVisualNodeStatus::key, CreateVisualNodeStatus::json_user_function);
    register_json_user_function(CreateVisualPlayerBulletCount::key, CreateVisualPlayerBulletCount::json_user_function);
    register_json_user_function(CreateVisualPlayerStatus::key, CreateVisualPlayerStatus::json_user_function);
    register_json_user_function(CreateWeaponCycleKeyBinding::key, CreateWeaponCycleKeyBinding::json_user_function);
    register_json_user_function(CreateWeaponCycle::key, CreateWeaponCycle::json_user_function);
    register_json_user_function(CreateWheel::key, CreateWheel::json_user_function);
    register_json_user_function(CreateWing::key, CreateWing::json_user_function);
    register_json_user_function(CreateYawPitchLookatNodes::key, CreateYawPitchLookatNodes::json_user_function);
    register_json_user_function(DefineWinnerConditionals::key, DefineWinnerConditionals::json_user_function);
    register_json_user_function(TryDeleteNode::key, TryDeleteNode::json_user_function);
    register_json_user_function(TryDeleteRootNode::key, TryDeleteRootNode::json_user_function);
    register_json_user_function(DeleteNode::key, DeleteNode::json_user_function);
    register_json_user_function(DeleteRootNodes::key, DeleteRootNodes::json_user_function);
    register_json_user_function(DeleteRootNode::key, DeleteRootNode::json_user_function);
    register_json_user_function(DeleteScheduledAdvanceTimes::key, DeleteScheduledAdvanceTimes::json_user_function);
    register_json_user_function(ExecuteInPhysicsThread::key, ExecuteInPhysicsThread::json_user_function);
    register_json_user_function(FillPixelRegionWithTexture::key, FillPixelRegionWithTexture::json_user_function);
    register_json_user_function(FocusedText::key, FocusedText::json_user_function);
    register_json_user_function(FollowNode::key, FollowNode::json_user_function);
    register_json_user_function(HudImage::key, HudImage::json_user_function);
    register_json_user_function(InvalidateAggregateRenderers::key, InvalidateAggregateRenderers::json_user_function);
    register_json_user_function(LoadPlayers::key, LoadPlayers::json_user_function);
    register_json_user_function(LookAtNode::key, LookAtNode::json_user_function);
    register_json_user_function(CreateOrthoCamera::key, CreateOrthoCamera::json_user_function);
    register_json_user_function(CreatePerspectiveCamera::key, CreatePerspectiveCamera::json_user_function);
    register_json_user_function(PlaybackTrack::key, PlaybackTrack::json_user_function);
    register_json_user_function(PlaybackWinnerTrack::key, PlaybackWinnerTrack::json_user_function);
    register_json_user_function(PlayerSetAimingGun::key, PlayerSetAimingGun::json_user_function);
    register_json_user_function(PlayerSetCanAim::key, PlayerSetCanAim::json_user_function);
    register_json_user_function(PlayerSetCanDrive::key, PlayerSetCanDrive::json_user_function);
    register_json_user_function(PlayerSetCanSelectBestWeapon::key, PlayerSetCanSelectBestWeapon::json_user_function);
    register_json_user_function(PlayerSetCanShoot::key, PlayerSetCanShoot::json_user_function);
    register_json_user_function(PlayerSetNode::key, PlayerSetNode::json_user_function);
    register_json_user_function(PlayerSetPathfindingWaypoints::key, PlayerSetPathfindingWaypoints::json_user_function);
    register_json_user_function(PlayerSetPlaybackWaypoints::key, PlayerSetPlaybackWaypoints::json_user_function);
    register_json_user_function(PlayerSetVehicleControlParameters::key, PlayerSetVehicleControlParameters::json_user_function);
    register_json_user_function(PlayerSetWaypoint::key, PlayerSetWaypoint::json_user_function);
    register_json_user_function(PlayersStats::key, PlayersStats::json_user_function);
    register_json_user_function(Preload::key, Preload::json_user_function);
    register_json_user_function(RecordTrackGpx::key, RecordTrackGpx::json_user_function);
    register_json_user_function(RecordTrack::key, RecordTrack::json_user_function);
    register_json_user_function(RegisterGeographicMapping::key, RegisterGeographicMapping::json_user_function);
    register_json_user_function(RemoveNodeNotAllowedToBeUnregistered::key, RemoveNodeNotAllowedToBeUnregistered::json_user_function);
    register_json_user_function(RenderableInstance::key, RenderableInstance::json_user_function);
    register_json_user_function(RespawnAllPlayers::key, RespawnAllPlayers::json_user_function);
    register_json_user_function(RootNodeInstance::key, RootNodeInstance::json_user_function);
    register_json_user_function(SceneToPercentageRegion::key, SceneToPercentageRegion::json_user_function);
    register_json_user_function(SceneToPixelRegion::key, SceneToPixelRegion::json_user_function);
    register_json_user_function(SceneToTexture::key, SceneToTexture::json_user_function);
    register_json_user_function(SetAnimationState::key, SetAnimationState::json_user_function);
    register_json_user_function(SetAvatarStyleUpdater::key, SetAvatarStyleUpdater::json_user_function);
    register_json_user_function(SetBackgroundColor::key, SetBackgroundColor::json_user_function);
    register_json_user_function(SetCameraCycle::key, SetCameraCycle::json_user_function);
    register_json_user_function(SetCamera::key, SetCamera::json_user_function);
    register_json_user_function(SetDesiredWeapon::key, SetDesiredWeapon::json_user_function);
    register_json_user_function(SetDirtmap::key, SetDirtmap::json_user_function);
    register_json_user_function(SetExternalsCreator::key, SetExternalsCreator::json_user_function);
    register_json_user_function(SetInventoryCapacity::key, SetInventoryCapacity::json_user_function);
    register_json_user_function(SetJumpStrength::key, SetJumpStrength::json_user_function);
    register_json_user_function(SetNodeBone::key, SetNodeBone::json_user_function);
    register_json_user_function(SetNodeHider::key, SetNodeHider::json_user_function);
    register_json_user_function(SetNodeRotation::key, SetNodeRotation::json_user_function);
    register_json_user_function(SetPreferredCarSpawner::key, SetPreferredCarSpawner::json_user_function);
    register_json_user_function(SetRaceIdentifierAndReloadHistory::key, SetRaceIdentifierAndReloadHistory::json_user_function);
    register_json_user_function(SetRigidBodyAlignToSurfaceRelaxation::key, SetRigidBodyAlignToSurfaceRelaxation::json_user_function);
    register_json_user_function(SetRigidBodyGrindPoint::key, SetRigidBodyGrindPoint::json_user_function);
    register_json_user_function(SetRigidBodyRevertSurfacePowerThreshold::key, SetRigidBodyRevertSurfacePowerThreshold::json_user_function);
    register_json_user_function(SetRigidBodyTarget::key, SetRigidBodyTarget::json_user_function);
    register_json_user_function(SetSkaterStyleUpdater::key, SetSkaterStyleUpdater::json_user_function);
    register_json_user_function(SetSkybox::key, SetSkybox::json_user_function);
    register_json_user_function(SetObjective::key, SetObjective::json_user_function);
    register_json_user_function(ResetSupplyDepotCooldowns::key, ResetSupplyDepotCooldowns::json_user_function);
    register_json_user_function(SetSoftLight::key, SetSoftLight::json_user_function);
    register_json_user_function(SetSpawnPoints::key, SetSpawnPoints::json_user_function);
    register_json_user_function(SetVip::key, SetVip::json_user_function);
    register_json_user_function(SpawnerSetPlayer::key, SpawnerSetPlayer::json_user_function);
    register_json_user_function(StartRace::key, StartRace::json_user_function);
    register_json_user_function(TeamSetWaypoint::key, TeamSetWaypoint::json_user_function);
    register_json_user_function(UiBackground::key, UiBackground::json_user_function);
    register_json_user_function(UiExhibit::key, UiExhibit::json_user_function);
    register_json_user_function(VisualNodeStatus3rd::key, VisualNodeStatus3rd::json_user_function);
    register_json_user_function(WithDeleteNodeMutex::key, WithDeleteNodeMutex::json_user_function);
    register_json_user_function(YplnUpdateBulletProperties::key, YplnUpdateBulletProperties::json_user_function);
    register_json_user_function(LoadKeyConfigurations::key, LoadKeyConfigurations::json_user_function);

    // Resources
    register_json_user_function(LoadOsmResource::key, LoadOsmResource::json_user_function);
    register_json_user_function(AddCubemap::key, AddCubemap::json_user_function);
    register_json_user_function(AddAudio::key, AddAudio::json_user_function);
    register_json_user_function(AddAudioSequence::key, AddAudioSequence::json_user_function);
    register_json_user_function(AddBlendMapTexture::key, AddBlendMapTexture::json_user_function);
    register_json_user_function(AddBvhResource::key, AddBvhResource::json_user_function);
    register_json_user_function(AddCompanionRenderable::key, AddCompanionRenderable::json_user_function);
    register_json_user_function(AddTextureAtlas::key, AddTextureAtlas::json_user_function);
    register_json_user_function(AddTextureDescriptor::key, AddTextureDescriptor::json_user_function);
    register_json_user_function(AppendFocuses::key, AppendFocuses::json_user_function);
    register_json_user_function(CreateBinaryXResource::key, CreateBinaryXResource::json_user_function);
    register_json_user_function(CreateBlendingXResource::key, CreateBlendingXResource::json_user_function);
    register_json_user_function(Downsample::key, Downsample::json_user_function);
    register_json_user_function(GenRay::key, GenRay::json_user_function);
    register_json_user_function(GenTriangleRays::key, GenTriangleRays::json_user_function);
    register_json_user_function(GenGrindLines::key, GenGrindLines::json_user_function);
    register_json_user_function(GenContourEdges::key, GenContourEdges::json_user_function);
    register_json_user_function(GenInstances::key, GenInstances::json_user_function);
    register_json_user_function(GenCompoundResource::key, GenCompoundResource::json_user_function);
    register_json_user_function(ImportBoneWeights::key, ImportBoneWeights::json_user_function);
    register_json_user_function(ObjResource::key, ObjResource::json_user_function);
    register_json_user_function(Repeat::key, Repeat::json_user_function);
    register_json_user_function(SaveTextureAtlasPng::key, SaveTextureAtlasPng::json_user_function);
    register_json_user_function(SetFocuses::key, SetFocuses::json_user_function);
    register_json_user_function(CreateSquareResource::key, CreateSquareResource::json_user_function);
    register_json_user_function(CreateGridResource::key, CreateGridResource::json_user_function);
    register_json_user_function(ModifyPhysicsMaterialTags::key, ModifyPhysicsMaterialTags::json_user_function);
    register_json_user_function(PrintResource::key, PrintResource::json_user_function);
    register_json_user_function(Echo::key, Echo::json_user_function);
    register_json_user_function(CreateTapButton::key, CreateTapButton::json_user_function);
    register_json_user_function(CreateAdditiveScreenConstraint::key, CreateAdditiveScreenConstraint::json_user_function);
    register_json_user_function(CreateConstantScreenConstraint::key, CreateConstantScreenConstraint::json_user_function);
    register_json_user_function(CreateFractionalScreenConstraint::key, CreateFractionalScreenConstraint::json_user_function);
    register_json_user_function(SetSurfaceContactInfo::key, SetSurfaceContactInfo::json_user_function);

    // Main
    register_json_user_function(ReloadScene::key, ReloadScene::json_user_function);
    register_json_user_function(ClearSelectionIds::key, ClearSelectionIds::json_user_function);

    // Misc
    register_json_user_function(Sleep::key, Sleep::json_user_function);
    register_json_user_function(ConstantParameter::key, ConstantParameter::json_user_function);
}

LoadScene::~LoadScene() = default;

void LoadScene::operator()(
    const std::list<std::string>* search_path,
    const std::string& script_filename,
    ThreadSafeString& next_scene_filename,
    NotifyingJsonMacroArguments& external_json_macro_arguments,
    std::atomic_size_t& num_renderings,
    bool verbose,
    SceneNodeResources& scene_node_resources,
    SurfaceContactDb& surface_contact_db,
    SceneConfig& scene_config,
    ButtonStates& button_states,
    CursorStates& cursor_states,
    CursorStates& scroll_wheel_states,
    UiFocus& ui_focus,
    LayoutConstraints& layout_constraints,
#ifndef __ANDROID__
    GLFWwindow& glfw_window,
#endif
    RenderLogicGallery& gallery,
    AssetReferences& asset_references,
    RenderableScenes& renderable_scenes)
{
    MacroLineExecutor::JsonUserFunction json_user_function = [&](
        const std::string& context,
        const MacroLineExecutor& macro_line_executor,
        const std::string& name,
        const JsonMacroArguments& arguments,
        JsonMacroArguments* local_json_macro_arguments)
    {
        auto renderable_scene = [&]() -> RenderableScene& {
            return renderable_scenes[context];
        };
        LoadSceneJsonUserFunctionArgs args{
            .name = name,
            .arguments = arguments,
            .renderable_scene = renderable_scene,
            .macro_line_executor = macro_line_executor,
            .external_json_macro_arguments = external_json_macro_arguments,
            .local_json_macro_arguments = local_json_macro_arguments,
            .scene_node_resources = scene_node_resources,
            .surface_contact_db = surface_contact_db,
            .scene_config = scene_config,
            .button_states = button_states,
            .cursor_states = cursor_states,
            .scroll_wheel_states = scroll_wheel_states,
            .ui_focus = ui_focus,
            .layout_constraints = layout_constraints,
#ifndef __ANDROID__
            .glfw_window = glfw_window,
#endif
            .num_renderings = num_renderings,
            .script_filename = script_filename,
            .next_scene_filename = next_scene_filename,
            .gallery = gallery,
            .asset_references = asset_references,
            .renderable_scenes = renderable_scenes};
        auto it = json_user_functions_.find(args.name);
        if (it == json_user_functions_.end()) {
            return false;
        }
        it->second(args);
        return true;
    };
    MacroLineExecutor lp2{
        macro_file_executor_,
        script_filename,
        search_path,
        json_user_function,
        "no_scene_specified",
        external_json_macro_arguments,
        verbose};
    macro_file_executor_(lp2, nullptr);
}
