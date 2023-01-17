#include "Load_Scene.hpp"
#include <Mlib/Macro_Executor/Macro_Line_Executor.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Constant_Parameter.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Containers/Add_To_Gallery.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Containers/Create_Scene.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Create_Additive_Screen_Constraint.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Create_Constant_Screen_Constraint.hpp>
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
#include <Mlib/Scene/Load_Scene_Functions/Instances/Lights/Create_Light_Only_Shadow.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Instances/Lights/Create_Light_With_Shadow.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Instances/Lights/Create_Light_Without_Shadow.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Instances/Lights/Set_Soft_Light.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Instances/Nodes/Add_Color_Style.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Instances/Nodes/Add_Node_Not_Allowed_To_Be_Unregistered.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Instances/Nodes/Append_Externals_Deleter.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Instances/Nodes/Clear_Node_Hider.hpp>
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
#include <Mlib/Scene/User_Function_Args.hpp>

using namespace Mlib;

LoadScene::LoadScene() {
    // Containers
    user_functions_.push_back(CreateScene::user_function);
    user_functions_.push_back(AddToGallery::user_function);

    // Instances
    user_functions_.push_back(AddColorStyle::user_function);
    user_functions_.push_back(AddNodeNotAllowedToBeUnregistered::user_function);
    user_functions_.push_back(AddToInventory::user_function);
    user_functions_.push_back(AddWeaponToInventory::user_function);
    user_functions_.push_back(AppendExternalsDeleter::user_function);
    user_functions_.push_back(BurnIn::user_function);
    user_functions_.push_back(ClearAbsoluteObserverAndNotifyDestroyed::user_function);
    user_functions_.push_back(ClearNodeHider::user_function);
    user_functions_.push_back(ClearNodesNotAllowedToBeUnregistered::user_function);
    user_functions_.push_back(ClearParameters::user_function);
    user_functions_.push_back(ClearRenderableInstance::user_function);
    user_functions_.push_back(ConsoleLog::user_function);
    user_functions_.push_back(Controls::user_function);
    user_functions_.push_back(Countdown::user_function);
    user_functions_.push_back(CreateAbsKeyBinding::user_function);
    user_functions_.push_back(CreateAvatarControllerIdleBinding::user_function);
    user_functions_.push_back(CreateAvatarControllerKeyBinding::user_function);
    user_functions_.push_back(CreateCameraKeyBinding::user_function);
    user_functions_.push_back(CreateCarControllerIdleBinding::user_function);
    user_functions_.push_back(CreateCarControllerKeyBinding::user_function);
    user_functions_.push_back(CreateCarController::user_function);
    user_functions_.push_back(CreateCheckPoints::user_function);
    user_functions_.push_back(CreateChildNode::user_function);
    user_functions_.push_back(CreateCrash::user_function);
    user_functions_.push_back(CreateDamageable::user_function);
    user_functions_.push_back(CreateDriverKeyBinding::user_function);
    user_functions_.push_back(CreateEngine::user_function);
    user_functions_.push_back(CreateExternals::user_function);
    user_functions_.push_back(CreateGunKeyBinding::user_function);
    user_functions_.push_back(CreateGun::user_function);
    user_functions_.push_back(CreateHeliController::user_function);
    user_functions_.push_back(CreateHumanAsAvatarController::user_function);
    user_functions_.push_back(CreateHumanAsCarController::user_function);
    user_functions_.push_back(CreateKeepOffsetFromCamera::user_function);
    user_functions_.push_back(CreateKeepOffsetFromMovable::user_function);
    user_functions_.push_back(CreateLightOnlyShadow::user_function);
    user_functions_.push_back(CreateLightWithoutShadow::user_function);
    user_functions_.push_back(CreateLightWithShadow::user_function);
    user_functions_.push_back(CreateParameterSetterLogic::user_function);
    user_functions_.push_back(CreatePlaneAsCarController::user_function);
    user_functions_.push_back(CreatePlaneControllerIdleBinding::user_function);
    user_functions_.push_back(CreatePlaneControllerKeyBinding::user_function);
    user_functions_.push_back(CreatePlaneController::user_function);
    user_functions_.push_back(CreatePlayer::user_function);
    user_functions_.push_back(CreateRelativeTransformer::user_function);
    user_functions_.push_back(CreateCopyRotation::user_function);
    user_functions_.push_back(CreateRelKeyBinding::user_function);
    user_functions_.push_back(CreateRigidCuboid::user_function);
    user_functions_.push_back(CreateRigidDisk::user_function);
    user_functions_.push_back(CreateRotor::user_function);
    user_functions_.push_back(CreateSceneSelectorLogic::user_function);
    user_functions_.push_back(CreateTabMenuLogic::user_function);
    user_functions_.push_back(CreateTankController::user_function);
    user_functions_.push_back(CreateVisualGlobalLog::user_function);
    user_functions_.push_back(CreateVisualNodeStatus::user_function);
    user_functions_.push_back(CreateVisualPlayerBulletCount::user_function);
    user_functions_.push_back(CreateVisualPlayerStatus::user_function);
    user_functions_.push_back(CreateWeaponCycleKeyBinding::user_function);
    user_functions_.push_back(CreateWeaponCycle::user_function);
    user_functions_.push_back(CreateWheel::user_function);
    user_functions_.push_back(CreateWing::user_function);
    user_functions_.push_back(CreateYawPitchLookatNodes::user_function);
    user_functions_.push_back(DefineWinnerConditionals::user_function);
    user_functions_.push_back(TryDeleteNode::user_function);
    user_functions_.push_back(TryDeleteRootNode::user_function);
    user_functions_.push_back(DeleteNode::user_function);
    user_functions_.push_back(DeleteRootNodes::user_function);
    user_functions_.push_back(DeleteRootNode::user_function);
    user_functions_.push_back(DeleteScheduledAdvanceTimes::user_function);
    user_functions_.push_back(ExecuteInPhysicsThread::user_function);
    user_functions_.push_back(FillPixelRegionWithTexture::user_function);
    user_functions_.push_back(FocusedText::user_function);
    user_functions_.push_back(FollowNode::user_function);
    user_functions_.push_back(HudImage::user_function);
    user_functions_.push_back(InvalidateAggregateRenderers::user_function);
    user_functions_.push_back(LoadPlayers::user_function);
    user_functions_.push_back(LookAtNode::user_function);
    user_functions_.push_back(CreateOrthoCamera::user_function);
    user_functions_.push_back(CreatePerspectiveCamera::user_function);
    user_functions_.push_back(PlaybackTrack::user_function);
    user_functions_.push_back(PlaybackWinnerTrack::user_function);
    user_functions_.push_back(PlayerSetAimingGun::user_function);
    user_functions_.push_back(PlayerSetCanAim::user_function);
    user_functions_.push_back(PlayerSetCanDrive::user_function);
    user_functions_.push_back(PlayerSetCanSelectBestWeapon::user_function);
    user_functions_.push_back(PlayerSetCanShoot::user_function);
    user_functions_.push_back(PlayerSetNode::user_function);
    user_functions_.push_back(PlayerSetPathfindingWaypoints::user_function);
    user_functions_.push_back(PlayerSetPlaybackWaypoints::user_function);
    user_functions_.push_back(PlayerSetVehicleControlParameters::user_function);
    user_functions_.push_back(PlayerSetWaypoint::user_function);
    user_functions_.push_back(PlayersStats::user_function);
    user_functions_.push_back(Preload::user_function);
    user_functions_.push_back(RecordTrackGpx::user_function);
    user_functions_.push_back(RecordTrack::user_function);
    user_functions_.push_back(RegisterGeographicMapping::user_function);
    user_functions_.push_back(RemoveNodeNotAllowedToBeUnregistered::user_function);
    user_functions_.push_back(RenderableInstance::user_function);
    user_functions_.push_back(RespawnAllPlayers::user_function);
    user_functions_.push_back(RootNodeInstance::user_function);
    user_functions_.push_back(SceneToPercentageRegion::user_function);
    user_functions_.push_back(SceneToPixelRegion::user_function);
    user_functions_.push_back(SceneToTexture::user_function);
    user_functions_.push_back(SetAnimationState::user_function);
    user_functions_.push_back(SetAvatarStyleUpdater::user_function);
    user_functions_.push_back(SetBackgroundColor::user_function);
    user_functions_.push_back(SetCameraCycle::user_function);
    user_functions_.push_back(SetCamera::user_function);
    user_functions_.push_back(SetDesiredWeapon::user_function);
    user_functions_.push_back(SetDirtmap::user_function);
    user_functions_.push_back(SetExternalsCreator::user_function);
    user_functions_.push_back(SetInventoryCapacity::user_function);
    user_functions_.push_back(SetJumpStrength::user_function);
    user_functions_.push_back(SetNodeBone::user_function);
    user_functions_.push_back(SetNodeHider::user_function);
    user_functions_.push_back(SetNodeRotation::user_function);
    user_functions_.push_back(SetPreferredCarSpawner::user_function);
    user_functions_.push_back(SetRaceIdentifierAndReloadHistory::user_function);
    user_functions_.push_back(SetRigidBodyAlignToSurfaceRelaxation::user_function);
    user_functions_.push_back(SetRigidBodyGrindPoint::user_function);
    user_functions_.push_back(SetRigidBodyRevertSurfacePowerThreshold::user_function);
    user_functions_.push_back(SetRigidBodyTarget::user_function);
    user_functions_.push_back(SetSkaterStyleUpdater::user_function);
    user_functions_.push_back(SetSkybox::user_function);
    user_functions_.push_back(SetObjective::user_function);
    user_functions_.push_back(ResetSupplyDepotCooldowns::user_function);
    user_functions_.push_back(SetSoftLight::user_function);
    user_functions_.push_back(SetSpawnPoints::user_function);
    user_functions_.push_back(SetVip::user_function);
    user_functions_.push_back(StartRace::user_function);
    user_functions_.push_back(TeamSetWaypoint::user_function);
    user_functions_.push_back(UiBackground::user_function);
    user_functions_.push_back(VisualNodeStatus3rd::user_function);
    user_functions_.push_back(WithDeleteNodeMutex::user_function);
    user_functions_.push_back(YplnUpdateBulletProperties::user_function);

    // Resources
    user_functions_.push_back(LoadOsmResource::user_function);
    user_functions_.push_back(AddCubemap::user_function);
    user_functions_.push_back(AddAudio::user_function);
    user_functions_.push_back(AddBlendMapTexture::user_function);
    user_functions_.push_back(AddBvhResource::user_function);
    user_functions_.push_back(AddCompanionRenderable::user_function);
    user_functions_.push_back(AddTextureAtlas::user_function);
    user_functions_.push_back(AddTextureDescriptor::user_function);
    user_functions_.push_back(AppendFocuses::user_function);
    user_functions_.push_back(CreateBinaryXResource::user_function);
    user_functions_.push_back(CreateBlendingXResource::user_function);
    user_functions_.push_back(Downsample::user_function);
    user_functions_.push_back(GenRay::user_function);
    user_functions_.push_back(GenTriangleRays::user_function);
    user_functions_.push_back(GenGrindLines::user_function);
    user_functions_.push_back(GenContourEdges::user_function);
    user_functions_.push_back(GenInstances::user_function);
    user_functions_.push_back(GenCompoundResource::user_function);
    user_functions_.push_back(ImportBoneWeights::user_function);
    user_functions_.push_back(LoadOsmResource::user_function);
    user_functions_.push_back(LoadOsmResource::user_function);
    user_functions_.push_back(ObjResource::user_function);
    user_functions_.push_back(Repeat::user_function);
    user_functions_.push_back(SaveTextureAtlasPng::user_function);
    user_functions_.push_back(SetFocuses::user_function);
    user_functions_.push_back(CreateSquareResource::user_function);
    user_functions_.push_back(CreateGridResource::user_function);
    user_functions_.push_back(ModifyPhysicsMaterialTags::user_function);
    user_functions_.push_back(PrintResource::user_function);
    user_functions_.push_back(Echo::user_function);
    user_functions_.push_back(CreateTapButton::user_function);
    user_functions_.push_back(CreateConstantScreenConstraint::user_function);
    user_functions_.push_back(CreateAdditiveScreenConstraint::user_function);
    user_functions_.push_back(SetSurfaceContactInfo::user_function);

    // Main
    user_functions_.push_back(ReloadScene::user_function);
    user_functions_.push_back(ClearSelectionIds::user_function);

    // Misc
    user_functions_.push_back(Sleep::user_function);
    user_functions_.push_back(ConstantParameter::user_function);
}

LoadScene::~LoadScene() = default;

void LoadScene::operator()(
    const std::list<std::string>& search_path,
    const std::string& script_filename,
    ThreadSafeString& next_scene_filename,
    SubstitutionMap& external_substitutions,
    std::atomic_size_t& num_renderings,
    bool verbose,
    RegexSubstitutionCache& rsc,
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
    RenderableScenes& renderable_scenes)
{
    MacroLineExecutor::UserFunction user_function = [&](
        const std::string& context,
        const std::function<FPath(const std::string&)>& fpath,
        const std::function<std::list<std::string>(const std::string&)>& fpathes,
        const MacroLineExecutor& macro_line_executor,
        const std::string& line,
        SubstitutionMap* local_substitutions) -> bool
    {
        auto renderable_scene = [&]() -> RenderableScene& {
            return renderable_scenes[context];
        };
        LoadSceneUserFunctionArgs args{
            .line = line,
            .renderable_scene = renderable_scene,
            .fpath = fpath,
            .fpathes = fpathes,
            .macro_line_executor = macro_line_executor,
            .external_substitutions = external_substitutions,
            .local_substitutions = local_substitutions,
            .rsc = rsc,
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
        search_path,
        user_function,
        "no_scene_specified",
        external_substitutions,
        verbose};
    macro_file_executor_(lp2, rsc);
}
