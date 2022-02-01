#include "Load_Scene.hpp"
#include <Mlib/Audio/Audio_Resources.hpp>
#include <Mlib/Env.hpp>
#include <Mlib/Geometry/Mesh/Load_Bvh.hpp>
#include <Mlib/Geometry/Mesh/Load_Mesh_Config.hpp>
#include <Mlib/Macro_Line_Executor.hpp>
#include <Mlib/Math/Fixed_Rodrigues.hpp>
#include <Mlib/Math/Pi.hpp>
#include <Mlib/Math/Rodrigues.hpp>
#include <Mlib/Physics/Actuators/Rigid_Body_Engine.hpp>
#include <Mlib/Physics/Advance_Times/Check_Points.hpp>
#include <Mlib/Physics/Advance_Times/Crash.hpp>
#include <Mlib/Physics/Advance_Times/Deleting_Damageable.hpp>
#include <Mlib/Physics/Advance_Times/Gun.hpp>
#include <Mlib/Physics/Advance_Times/Movable_Logger.hpp>
#include <Mlib/Physics/Advance_Times/Movables/Follow_Movable.hpp>
#include <Mlib/Physics/Advance_Times/Movables/Keep_Offset_Movable.hpp>
#include <Mlib/Physics/Advance_Times/Movables/Look_At_Movable.hpp>
#include <Mlib/Physics/Advance_Times/Movables/Pitch_Look_At_Node.hpp>
#include <Mlib/Physics/Advance_Times/Movables/Relative_Transformer.hpp>
#include <Mlib/Physics/Advance_Times/Movables/Rigid_Body_Playback.hpp>
#include <Mlib/Physics/Advance_Times/Movables/Wheel.hpp>
#include <Mlib/Physics/Advance_Times/Movables/Yaw_Pitch_Look_At_Nodes.hpp>
#include <Mlib/Physics/Advance_Times/Rigid_Body_Recorder.hpp>
#include <Mlib/Physics/Advance_Times/Rigid_Body_Recorder_Gpx.hpp>
#include <Mlib/Physics/Collision/Collidable_Mode.hpp>
#include <Mlib/Physics/Containers/Game_History.hpp>
#include <Mlib/Physics/Physics_Engine.hpp>
#include <Mlib/Physics/Physics_Loop.hpp>
#include <Mlib/Physics/Rigid_Body/Rigid_Body_Vehicle.hpp>
#include <Mlib/Physics/Rigid_Body/Rigid_Primitives.hpp>
#include <Mlib/Physics/Vehicle_Controllers/Car_Controller.hpp>
#include <Mlib/Physics/Vehicle_Controllers/Human_As_Car_Controller.hpp>
#include <Mlib/Physics/Vehicle_Controllers/Tank_Controller.hpp>
#include <Mlib/Players/Advance_Times/Game_Logic.hpp>
#include <Mlib/Players/Advance_Times/Player.hpp>
#include <Mlib/Players/Containers/Players.hpp>
#include <Mlib/Players/Mlib_Pod_Bot/Set_Pod_Bot_Game_Mode.hpp>
#include <Mlib/Players/Mlib_Pod_Bot/Set_Pod_Bot_Way_Points.hpp>
#include <Mlib/Regex.hpp>
#include <Mlib/Regex_Select.hpp>
#include <Mlib/Render/Cameras/Generic_Camera.hpp>
#include <Mlib/Render/Key_Bindings/Absolute_Movable_Idle_Binding.hpp>
#include <Mlib/Render/Key_Bindings/Absolute_Movable_Key_Binding.hpp>
#include <Mlib/Render/Key_Bindings/Camera_Key_Binding.hpp>
#include <Mlib/Render/Key_Bindings/Car_Controller_Key_Binding.hpp>
#include <Mlib/Render/Key_Bindings/Gun_Key_Binding.hpp>
#include <Mlib/Render/Key_Bindings/Relative_Movable_Key_Binding.hpp>
#include <Mlib/Render/Key_Bindings/Weapon_Inventory_Key_Binding.hpp>
#include <Mlib/Render/Render_Logics/Clear_Mode.hpp>
#include <Mlib/Render/Render_Logics/Controls_Logic.hpp>
#include <Mlib/Render/Render_Logics/Countdown_Logic.hpp>
#include <Mlib/Render/Render_Logics/Dirtmap_Logic.hpp>
#include <Mlib/Render/Render_Logics/Fill_Pixel_Region_With_Texture_Logic.hpp>
#include <Mlib/Render/Render_Logics/Focused_Text_Logic.hpp>
#include <Mlib/Render/Render_Logics/Lightmap_Logic.hpp>
#include <Mlib/Render/Render_Logics/Main_Menu_Background_Logic.hpp>
#include <Mlib/Render/Render_Logics/Pause_On_Lose_Focus_Logic.hpp>
#include <Mlib/Render/Render_Logics/Read_Pixels_Logic.hpp>
#include <Mlib/Render/Render_Logics/Render_Logics.hpp>
#include <Mlib/Render/Render_Logics/Render_To_Pixel_Region_Logic.hpp>
#include <Mlib/Render/Render_Logics/Render_To_Texture_Logic.hpp>
#include <Mlib/Render/Render_Logics/Resource_Update_Cycle.hpp>
#include <Mlib/Render/Render_Logics/Skybox_Logic.hpp>
#include <Mlib/Render/Rendering_Resources.hpp>
#include <Mlib/Render/Resources/Binary_X_Resource.hpp>
#include <Mlib/Render/Resources/Blending_X_Resource.hpp>
#include <Mlib/Render/Resources/Mhx2_File_Resource.hpp>
#include <Mlib/Render/Resources/Obj_File_Resource.hpp>
#include <Mlib/Render/Resources/Osm_Map_Resource.hpp>
#include <Mlib/Render/Resources/Osm_Map_Resource/Osm_Resource_Config.hpp>
#include <Mlib/Render/Resources/Osm_Map_Resource/Parsed_Resource_Name.hpp>
#include <Mlib/Render/Resources/Osm_Map_Resource/Road_Type.hpp>
#include <Mlib/Render/Resources/Osm_Map_Resource/Terrain_Type.hpp>
#include <Mlib/Render/Resources/Osm_Map_Resource/Wayside_Resource_Names.hpp>
#include <Mlib/Render/Resources/Square_Resource.hpp>
#include <Mlib/Render/Selected_Cameras.hpp>
#include <Mlib/Render/Ui/Button_Press.hpp>
#include <Mlib/Scene/Animation/Avatar_Animation_Updater.hpp>
#include <Mlib/Scene/Audio/Engine_Audio.hpp>
#include <Mlib/Scene/Linker.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Add_Weapon_To_Inventory.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Append_Externals_Deleter.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Create_Abs_Key_Binding.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Create_Avatar_Controller_Idle_Binding.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Create_Avatar_Controller_Key_Binding.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Create_Car_Controller.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Create_Car_Controller_Idle_Binding.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Create_Car_Controller_Key_Binding.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Create_Child_Node.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Create_Driver_Key_Binding.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Create_Externals.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Create_Gun_Key_Binding.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Create_Heli_Controller.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Create_Human_As_Avatar_Controller.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Create_Human_As_Car_Controller.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Create_Player.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Create_Rel_Key_Binding.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Create_Rigid_Cuboid.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Create_Rigid_Disk.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Create_Rotor.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Create_Tab_Menu_Logic.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Create_Tank_Controller.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Create_Visual_Node_Status.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Create_Weapon_Inventory.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Create_Wheel.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Create_Yaw_Pitch_Lookat_Nodes.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Equip_Weapon.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Load_Players.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Player_Set_Node.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Set_Externals_Creator.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Set_Preferred_Car_Spawner.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Set_Rigid_Body_Target.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Set_Spawn_Points.hpp>
#include <Mlib/Scene/Render_Logics/Hud_Image_Logic.hpp>
#include <Mlib/Scene/Render_Logics/Key_Bindings.hpp>
#include <Mlib/Scene/Render_Logics/Parameter_Setter_Logic.hpp>
#include <Mlib/Scene/Render_Logics/Players_Stats_Logic.hpp>
#include <Mlib/Scene/Render_Logics/Scene_Selector_Logic.hpp>
#include <Mlib/Scene/Render_Logics/Visual_Global_Log.hpp>
#include <Mlib/Scene/Render_Logics/Visual_Movable_3rd_Logger.hpp>
#include <Mlib/Scene/Render_Logics/Visual_Movable_Logger.hpp>
#include <Mlib/Scene/Renderable_Scene.hpp>
#include <Mlib/Scene_Graph/Base_Log.hpp>
#include <Mlib/Scene_Graph/Driving_Direction.hpp>
#include <Mlib/Scene_Graph/Log_Entry_Severity.hpp>
#include <Mlib/Scene_Graph/Scene.hpp>
#include <Mlib/Scene_Graph/Scene_Node_Resources.hpp>
#include <Mlib/Scene_Graph/Style.hpp>
#include <Mlib/Strings/From_Number.hpp>
#include <Mlib/Strings/String.hpp>
#include <Mlib/Strings/Trim.hpp>
#include <filesystem>
#include <fstream>
#include <regex>
#include <stb_image/stb_image_load.hpp>
#include <stb_image/stb_image_write.h>

namespace fs = std::filesystem;

using namespace Mlib;

FixedArray<float, 3> parse_position(
    const TransformationMatrix<double, 3>* inverse_geographic_coordinates,
    const std::string& x_str,
    const std::string& y_str,
    const std::string& z_str)
{
    static const DECLARE_REGEX(re, "^([\\d.-]+)_deg");
    Mlib::re::smatch match_x;
    Mlib::re::smatch match_y;
    bool mx = Mlib::re::regex_match(x_str, match_x, re);
    bool my = Mlib::re::regex_match(y_str, match_y, re);
    if (mx != my) {
        throw std::runtime_error("Inconsistent positions: " + x_str + ", " + y_str);
    }
    if (mx) {
        if (inverse_geographic_coordinates == nullptr) {
            throw std::runtime_error("World coordinates not defined");
        }
        return inverse_geographic_coordinates->transform(
            FixedArray<double, 3>{
                safe_stof(match_y[1].str()),
                safe_stof(match_x[1].str()),
                safe_stof(z_str)}).casted<float>();
    } else {
        return FixedArray<float, 3>{
            safe_stof(x_str),
            safe_stof(y_str),
            safe_stof(z_str)};
    }
}

LoadScene::LoadScene() {
    user_functions_.push_back(CreatePlayer::user_function);
    user_functions_.push_back(LoadPlayers::user_function);
    user_functions_.push_back(CreateCarController::user_function);
    user_functions_.push_back(CreateHeliController::user_function);
    user_functions_.push_back(CreateHumanAsAvatarController::user_function);
    user_functions_.push_back(CreateHumanAsCarController::user_function);
    user_functions_.push_back(CreateTankController::user_function);
    user_functions_.push_back(CreateWheel::user_function);
    user_functions_.push_back(CreateRotor::user_function);
    user_functions_.push_back(CreateRigidCuboid::user_function);
    user_functions_.push_back(CreateRigidDisk::user_function);
    user_functions_.push_back(SetRigidBodyTarget::user_function);
    user_functions_.push_back(CreateYawPitchLookatNodes::user_function);
    user_functions_.push_back(CreateDriverKeyBinding::user_function);
    user_functions_.push_back(CreateAbsKeyBinding::user_function);
    user_functions_.push_back(CreateCarControllerKeyBinding::user_function);
    user_functions_.push_back(CreateAvatarControllerIdleBinding::user_function);
    user_functions_.push_back(CreateAvatarControllerKeyBinding::user_function);
    user_functions_.push_back(CreateWeaponInventory::user_function);
    user_functions_.push_back(AddWeaponToInventory::user_function);
    user_functions_.push_back(EquipWeapon::user_function);
    user_functions_.push_back(SetPreferredCarSpawner::user_function);
    user_functions_.push_back(SetSpawnPoints::user_function);
    user_functions_.push_back(CreateTabMenuLogic::user_function);
    user_functions_.push_back(CreateGunKeyBinding::user_function);
    user_functions_.push_back(PlayerSetNode::user_function);
    user_functions_.push_back(SetExternalsCreator::user_function);
    user_functions_.push_back(CreateExternals::user_function);
    user_functions_.push_back(AppendExternalsDeleter::user_function);
    user_functions_.push_back(CreateChildNode::user_function);
    user_functions_.push_back(CreateCarControllerIdleBinding::user_function);
    user_functions_.push_back(CreateRelKeyBinding::user_function);
    user_functions_.push_back(CreateVisualNodeStatus::user_function);
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
    static const DECLARE_REGEX(osm_resource_reg,
        "^\\s*osm_resource\\s+([\\s\\S]+)");
    static const DECLARE_REGEX(obj_resource_reg,
        "^\\s*obj_resource"
        "\\s+name=([\\w-. \\(\\)/+-]+)"
        "\\s+filename=([\\w+-. \\(\\)/\\\\:]+)"
        "\\s+position=([\\w+-.]+)\\s+([\\w+-.]+)\\s+([\\w+-.]+)"
        "\\s+rotation=([\\w+-.]+)\\s+([\\w+-.]+)\\s+([\\w+-.]+)"
        "\\s+scale=([\\w+-.]+)\\s+([\\w+-.]+)\\s+([\\w+-.]+)"
        "(?:\\s+distances=([\\w+-.]+)\\s+([\\w+-.]+))?"
        "\\s+is_small=(0|1)"
        "\\s+blend_mode=(off|binary|semi_continuous|continuous)"
        "\\s+alpha_distances=([\\w+-.]+)\\s+([\\w+-.]+)\\s+([\\w+-.]+)\\s+([\\w+-.]+)"
        "\\s+cull_faces=(0|1)"
        "\\s+occluded_type=(off|color|depth)"
        "\\s+occluder_type=(off|white|black)"
        "\\s+occluded_by_black=(0|1)"
        "(?:\\s+is_black=(0|1))?"
        "\\s+aggregate_mode=(off|once|sorted|instances_once|instances_sorted)"
        "\\s+transformation_mode=(all|position|position_lookat|position_yangle)"
        "(?:\\s+triangle_tangent_error_behavior=(zero|warn|raise))?"
        "(\\s+no_werror)?$");
    static const DECLARE_REGEX(gen_triangle_rays_reg,
        "^\\s*gen_triangle_rays"
        "\\s+name=([\\w+-.]+)"
        "\\s+npoints=([\\w+-.]+)"
        "\\s+lengths=([\\w+-.]+)\\s+([\\w+-.]+)\\s+([\\w+-.]+)"
        "\\s+delete_triangles=(0|1)$");
    static const DECLARE_REGEX(gen_ray_reg,
        "^\\s*gen_ray name=([\\w+-.]+)"
        "\\s+from=([\\w+-.]+)\\s+([\\w+-.]+)\\s+([\\w+-.]+)"
        "\\s+to=([\\w+-.]+)\\s+([\\w+-.]+)\\s+([\\w+-.]+)$");
    static const DECLARE_REGEX(downsample_reg,
        "^\\s*downsample"
        "\\s+name=([/\\w+-.]+)"
        "\\s+factor=(\\d+)$");
    static const DECLARE_REGEX(import_bone_weights_reg,
        "^\\s*import_bone_weights"
        "\\s+destination=([/\\w+-.]+)"
        "\\s+source=([/\\w+-.]+)"
        "\\s+max_distance=([\\w+-.]+)$");
    static const DECLARE_REGEX(square_resource_reg,
        "^\\s*square_resource"
        "\\s+name=([\\w+-.]+)"
        "\\s+texture_filename=(#?[\\w-.\\(\\)/+-]+)"
        "\\s+min=([\\w+-.]+)\\s+([\\w+-.]+)"
        "\\s+max=([\\w+-.]+)\\s+([\\w+-.]+)"
        "(?:\\s+distances=([\\w+-.]+)\\s+([\\w+-.]+))?"
        "\\s+is_small=(0|1)"
        "\\s+occluded_type=(off|color|depth)"
        "\\s+occluder_type=(off|white|black)"
        "\\s+occluded_by_black=(0|1)"
        "\\s+ambience=([\\w+-.]+)\\s+([\\w+-.]+)\\s+([\\w+-.]+)"
        "\\s+blend_mode=(off|binary|semi_continuous|continuous)"
        "(?:\\s+depth_func=(less_equal|less|equal))?"
        "\\s+alpha_distances=([\\w+-.]+)\\s+([\\w+-.]+)\\s+([\\w+-.]+)\\s+([\\w+-.]+)"
        "\\s+cull_faces=(0|1)"
        "(?:\\s+rotation=([\\w+-.]+)\\s+([\\w+-.]+)\\s+([\\w+-.]+))?"
        "(?:\\s+translation=([\\w+-.]+)\\s+([\\w+-.]+)\\s+([\\w+-.]+))?"
        "\\s+aggregate_mode=(off|once|sorted|instances_once|instances_sorted)"
        "\\s+transformation_mode=(all|position|position_lookat|position_yangle)"
        "(?:\\s+number_of_frames=(\\d+))?"
        "(?:\\s+billboards=([\\s\\S]*))?$");
    static const DECLARE_REGEX(blending_x_resource_reg,
        "^\\s*blending_x_resource"
        "\\s+name=([\\w+-.]+)"
        "\\s+texture_filename=([\\w+-. \\(\\)/\\\\:]+)"
        "\\s+min=([\\w+-.]+)\\s+([\\w+-.]+)"
        "\\s+max=([\\w+-.]+)\\s+([\\w+-.]+)$");
    static const DECLARE_REGEX(binary_x_resource_reg,
        "^\\s*binary_x_resource"
        "\\s+name=([\\w+-.]+)"
        "\\s+texture_filename_0=(#?[\\w-.\\(\\)/+-]+)"
        "\\s+texture_filename_90=(#?[\\w-.\\(\\)/+-]+)"
        "\\s+min=([\\w+-.]+)\\s+([\\w+-.]+)"
        "\\s+max=([\\w+-.]+)\\s+([\\w+-.]+)"
        "(?:\\s+distances=([\\w+-.]+)\\s+([\\w+-.]+))?"
        "\\s+is_small=(0|1)"
        "\\s+occluded_type=(off|color|depth)"
        "\\s+occluder_type=(off|white|black)"
        "\\s+occluded_by_black=(0|1)"
        "\\s+ambience=([\\w+-.]+)\\s+([\\w+-.]+)\\s+([\\w+-.]+)"
        "\\s+blend_mode=(off|binary|semi_continuous|continuous)"
        "\\s+alpha_distances=([\\w+-.]+)\\s+([\\w+-.]+)\\s+([\\w+-.]+)\\s+([\\w+-.]+)"
        "\\s+cull_faces=(0|1)"
        "\\s+aggregate_mode=(off|once|sorted|instances_once|instances_sorted)"
        "\\s+transformation_mode=(all|position|position_lookat|position_yangle)$");
    static const DECLARE_REGEX(root_node_instance_reg,
        "^\\s*root_node_instance"
        "\\s+type=(aggregate|instances|static|dynamic)"
        "\\s+name=([\\w+-.]+)"
        "\\s+position=([\\w+-.]+)\\s+([\\w+-.]+)\\s+([\\w+-.]+)"
        "\\s+rotation=([\\w+-.]+)\\s+([\\w+-.]+)\\s+([\\w+-.]+)"
        "(?:\\s+scale=([\\w+-.]+))?$");
    static const DECLARE_REGEX(delete_root_node_reg,
        "^\\s*delete_root_node"
        "\\s+name=([\\w+-.]+)$");
    static const DECLARE_REGEX(delete_root_nodes_reg,
        "^\\s*delete_root_nodes"
        "\\s+regex=(.*)$");
    static const DECLARE_REGEX(delete_scheduled_advance_times_reg,
        "^\\s*delete_scheduled_advance_times$");
    static const DECLARE_REGEX(renderable_instance_reg,
        "^\\s*renderable_instance"
        "\\s+name=([\\w+-.]+)"
        "\\s+node=([\\w+-.]+)"
        "\\s+resource=([\\w-. \\(\\)/+-]+)"
        "(?:\\s+regex=(.*))?$");
    static const DECLARE_REGEX(add_companion_renderable_reg,
        "^\\s*add_companion_renderable"
        "\\s+resource=([\\w+-.]+)"
        "\\s+companion_resource=([\\w+-.]+)"
        "(?:\\s+regex=(.*))?$");
    static const DECLARE_REGEX(register_geographic_mapping_reg,
        "^\\s*register_geographic_mapping"
        "\\s+name=([\\w+-.]+)"
        "\\s+node=([\\w+-.]+)"
        "\\s+resource=([\\w-. \\(\\)/+-]+)");
    static const DECLARE_REGEX(gun_reg,
        "^\\s*gun"
        "\\s+node=([\\w+-.]+)"
        "\\s+parent_rigid_body_node=([\\w+-.]+)"
        "\\s+punch_angle_node=([\\w+-.]+)"
        "\\s+cool_down=([\\w+-.]+)"
        "\\s+renderable=([\\w-. \\(\\)/+-]+)"
        "\\s+hitbox=([\\w-. \\(\\)/+-]+)"
        "\\s+mass=([\\w+-.]+)"
        "\\s+velocity=([\\w+-.]+)"
        "\\s+lifetime=([\\w+-.]+)"
        "\\s+damage=([\\w+-.]+)"
        "\\s+size=([\\w+-.]+)\\s+([\\w+-.]+)\\s+([\\w+-.]+)"
        "\\s+punch_angle=([\\w+-.]+)$");
    static const DECLARE_REGEX(damageable_reg,
        "^\\s*damageable node=([\\w+-.]+)"
        "\\s+health=([\\w+-.]+)$");
    static const DECLARE_REGEX(crash_reg,
        "^\\s*crash node=([\\w+-.]+)"
        "\\s+damage=([\\w+-.]+)$");
    static const DECLARE_REGEX(relative_transformer_reg,
        "^\\s*relative_transformer"
        "\\s+node=\\s*([\\w+-.]+)"
        "(?:\\s+v=\\s*([\\w+-.]+)\\s+([\\w+-.]+)\\s+([\\w+-.]+))?"
        "(?:\\s+w=\\s*([\\w+-.]+)\\s+([\\w+-.]+)\\s+([\\w+-.]+))?$");
    static const DECLARE_REGEX(create_engine_reg,
        "^\\s*create_engine"
        "\\s+rigid_body=([\\w+-.]+)"
        "\\s+name=([\\w+-.]+)"
        "\\s+angular_vels=([ \\w+-.]+)"
        "\\s+powers=([ \\w+-.]+)"
        "\\s+gear_ratios=([ \\w+-.]+)"
        "(?:\\s+hand_brake_pulled=(0|1))?"
        "(?:\\s+audio=([\\w-. \\(\\)/+-]+))?");
    static const DECLARE_REGEX(clear_renderable_instance_reg,
        "^\\s*clear_renderable_instance"
        "\\s+node=([\\w+-.]+)"
        "\\s+name=([\\w+-.]+)$");
    static const DECLARE_REGEX(clear_absolute_observer_reg,
        "^\\s*clear_absolute_observer"
        "\\s+node=([\\w+-.]+)$");
    static const DECLARE_REGEX(player_set_aiming_gun_reg,
        "^\\s*player_set_aiming_gun"
        "\\s+player_name=([\\w+-.]+)"
        "\\s+yaw_node=([\\w+-.]+)"
        "\\s+gun_node=([\\w+-.]*)$");
    static const DECLARE_REGEX(player_set_surface_power_reg,
        "^\\s*player_set_surface_power"
        "\\s+player_name=([\\w+-.]+)"
        "\\s+forward=([\\w+-.]+)"
        "\\s+backward=([\\w+-.]*)$");
    static const DECLARE_REGEX(player_set_waypoint_reg,
        "^\\s*player_set_waypoint"
        "\\s+player_name=([\\w+-.]+)"
        "\\s+position=([\\w+-.]*) ([\\w+-.]*) ([\\w+-.]*)$");
    static const DECLARE_REGEX(team_set_waypoint_reg,
        "^\\s*team_set_waypoint"
        "\\s+team-name=([\\w+-.]+)"
        "\\s+position=([\\w+-.]*) ([\\w+-.]*) ([\\w+-.]*)$");
    static const DECLARE_REGEX(camera_key_binding_reg,
        "^\\s*camera_key_binding"
        "\\s+key=([\\w+-.]+)"
        "\\s+gamepad_button=([\\w+-.]*)"
        "(?:\\s+joystick_digital_axis=([\\w+-.]*)"
        "\\s+joystick_digital_axis_sign=([\\w+-.]+))?$");
    static const DECLARE_REGEX(abs_idle_binding_reg,
        "^\\s*abs_idle_binding"
        "\\s+node=([\\w+-.]+)"
        "\\s+tires_z=([\\w+-.]+)\\s+([\\w+-.]+)\\s+([\\w+-.]+)$");
    static const DECLARE_REGEX(weapon_inventory_key_binding_reg,
        "^\\s*weapon_inventory_key_binding"
        "\\s+node=([\\w+-.]+)"
        "\\s+key=([\\w+-.]+)"
        "(?:\\s+gamepad_button=([\\w+-.]*))?"
        "\\s+joystick_digital_axis=([\\w+-.]*)"
        "\\s+joystick_digital_axis_sign=([\\w+-.]+)"
        "(?:\\s+scroll_wheel_axis=(0|1))?"
        "(?:\\s+scroll_wheel_sign_and_scale=([\\w+-.]+))?"
        "\\s+weapon_increment=([\\d-]+)$");
    static const DECLARE_REGEX(console_log_reg,
        "^\\s*console_log"
        "\\s+node=([\\w+-.]+)"
        "\\s+format=(\\d+)$");
    static const DECLARE_REGEX(visual_global_log_reg,
        "^\\s*visual_global_log"
        "\\s+ttf_file=([\\w-. \\(\\)/+-]+)"
        "\\s+position=([\\w+-.]+)\\s+([\\w+-.]+)"
        "\\s+font_height=([\\w+-.]+)"
        "\\s+line_distance=([\\w+-.]+)"
        "\\s+nentries=([\\d+]+)"
        "\\s+severity=(info|critical)$");
    static const DECLARE_REGEX(visual_node_status_3rd_reg,
        "^\\s*visual_node_status_3rd"
        "\\s+node=([\\w+-.]+)"
        "\\s+format=(\\d+)"
        "\\s+ttf_file=([\\w-. \\(\\)/+-]+)"
        "\\s+offset=([\\w+-.]+)\\s+([\\w+-.]+)"
        "\\s+font_height=([\\w+-.]+)"
        "\\s+line_distance=([\\w+-.]+)$");
    static const DECLARE_REGEX(focused_text_reg,
        "^\\s*focused_text"
        "\\s+ttf_file=([\\w-. \\(\\)/+-]+)"
        "\\s+position=([\\w+-.]+)\\s+([\\w+-.]+)"
        "\\s+font_height=([\\w+-.]+)"
        "\\s+line_distance=([\\w+-.]+)"
        "\\s+focus_mask=(none|base|menu|loading|countdown_any|scene|game_over|always)"
        "\\s+text=(.*)$");
    static const DECLARE_REGEX(countdown_reg,
        "^\\s*countdown"
        "\\s+ttf_file=([\\w-. \\(\\)/+-]+)"
        "\\s+position=([\\w+-.]+)\\s+([\\w+-.]+)"
        "\\s+font_height=([\\w+-.]+)"
        "\\s+line_distance=([\\w+-.]+)"
        "\\s+nseconds=([\\w+-.]+)$");
    static const DECLARE_REGEX(players_stats_reg,
        "^\\s*players_stats"
        "\\s+ttf_file=([\\w-. \\(\\)/+-]+)"
        "\\s+position=([\\w+-.]+)\\s+([\\w+-.]+)"
        "\\s+font_height=([\\w+-.]+)"
        "\\s+line_distance=([\\w+-.]+)"
        "\\s+score_board=(\\d+)$");
    static const DECLARE_REGEX(create_scene_reg,
        "^\\s*create_scene"
        "\\s+name=([\\w+-.]+)"
        "\\s+z_order=([\\d-]+)"
        "\\s+fly=(0|1)"
        "\\s+rotate=(0|1)"
        "\\s+print_gamepad_buttons=(0|1)"
        "\\s+depth_fog=(0|1)"
        "\\s+low_pass=(0|1)"
        "\\s+high_pass=(0|1)"
        "\\s+vfx=(0|1)"
        "\\s+with_dirtmap=(0|1)"
        "\\s+with_skybox=(0|1)"
        "\\s+with_flying_logic=(0|1)"
        "\\s+with_pod_bot=(0|1)"
        "\\s+clear_mode=(off|color|depth|color_and_depth)"
        "\\s+max_tracks=(\\d+)$");
    static const DECLARE_REGEX(scene_selector_reg,
        "^\\s*scene_selector"
        "\\s+id=([\\w+-.]+)"
        "\\s+title=([\\w+-. ]*)"
        "\\s+ttf_file=([\\w-. \\(\\)/+-]+)"
        "\\s+position=([\\w+-.]+)\\s+([\\w+-.]+)"
        "\\s+font_height=([\\w+-.]+)"
        "\\s+line_distance=([\\w+-.]+)"
        "\\s+scene_files=([\\s\\w+-.\\(\\)/:=%]+)$");
    static const DECLARE_REGEX(scene_to_texture_reg,
        "^\\s*scene_to_texture"
        "\\s+texture_name=([\\w+-.]+)"
        "\\s+update=(once|always)"
        "\\s+size=([\\w+-.]+)\\s+([\\w+-.]+)"
        "\\s+focus_mask=(none|base|menu|loading|countdown_any|scene|game_over|always)"
        "\\s+submenu=(\\w*)$");
    static const DECLARE_REGEX(fill_pixel_region_with_texture_reg,
        "^\\s*fill_pixel_region_with_texture"
        "\\s+source_scene=([\\w+-.]+)"
        "\\s+texture_name=([\\w+-.]+)"
        "\\s+update=(once|always)"
        "\\s+position=([\\w+-.]+)\\s+([\\w+-.]+)"
        "\\s+size=([\\w+-.]+)\\s+([\\w+-.]+)"
        "\\s+focus_mask=(none|base|menu|loading|countdown_any|scene|game_over|always)"
        "\\s+submenu=(\\w*)$");
    static const DECLARE_REGEX(scene_to_pixel_region_reg,
        "^\\s*scene_to_pixel_region"
        "\\s+target_scene=([\\w+-.]+)"
        "\\s+position=([\\w+-.]+)\\s+([\\w+-.]+)"
        "\\s+size=([\\w+-.]+)\\s+([\\w+-.]+)"
        "\\s+focus_mask=(none|base|menu|loading|countdown_any|scene|game_over|always)"
        "\\s+submenu=(\\w*)$");
    static const DECLARE_REGEX(controls_reg,
        "^\\s*controls"
        "\\s+id=([\\w+-.]+)"
        "\\s+title=([\\w+-. ]*)"
        "\\s+gamepad_texture=(#?[\\w-. \\(\\)/+-]+)"
        "\\s+position=([\\w+-.]+)\\s+([\\w+-.]+)"
        "\\s+size=([\\w+-.]+)\\s+([\\w+-.]+)$");
    static const DECLARE_REGEX(clear_parameters_reg,
        "^\\s*clear_parameters$");
    static const DECLARE_REGEX(parameter_setter_reg,
        "^\\s*parameter_setter"
        "\\s+id=([\\w+-.]+)"
        "\\s+title=([\\w+-. ]*)"
        "\\s+ttf_file=([\\w-. \\(\\)/+-]+)"
        "\\s+position=([\\w+-.]+)\\s+([\\w+-.]+)"
        "\\s+font_height=([\\w+-.]+)"
        "\\s+line_distance=([\\w+-.]+)"
        "\\s+default=([\\d]+)"
        "\\s+on_init=([\\w+-.:= ]*)"
        "\\s+on_change=([\\w+-.:= ]*)"
        "\\s+parameters=([\\s\\S]*)$");
    static const DECLARE_REGEX(set_renderable_style_reg,
        "^\\s*set_renderable_style"
        "\\s+selector=([^\\r\\n]*)\\r?\\n"
        "\\s*node=([\\w+-.]+)"
        "\\s+ambience=([\\w+-.]+)\\s+([\\w+-.]+)\\s+([\\w+-.]+)"
        "\\s+diffusivity=([\\w+-.]+)\\s+([\\w+-.]+)\\s+([\\w+-.]+)"
        "\\s+specularity=([\\w+-.]+)\\s+([\\w+-.]+)\\s+([\\w+-.]+)"
        "(?:\\s+animation_name=([\\w+-.]*))?"
        "(?:\\s+animation_loop_begin=([\\w+-.]+))?"
        "(?:\\s+animation_loop_end=([\\w+-.]+))?"
        "(?:\\s+animation_loop_time=([\\w+-.]+))?$");
    static const DECLARE_REGEX(set_avatar_style_updater_reg,
        "^\\s*set_avatar_style_updater"
        "\\s+avatar_node=([\\w+-.]*)"
        "\\s+gun_node=([\\w+-.]+)"
        "\\s+resource_wo_gun=([\\w+-.]+)"
        "\\s+resource_w_gun=([\\w+-.]+)$");
    static const DECLARE_REGEX(add_bvh_resource_reg,
        "^\\s*add_bvh_resource"
        "\\s+name=([\\w+-.]+)"
        "\\s+filename=([\\w+-. \\(\\)/\\\\:]+)"
        "\\s+smooth_radius=([\\w+-.]+)"
        "\\s+smooth_alpha=([\\w+-.]+)"
        "\\s+periodic=(0|1)$");
    static const DECLARE_REGEX(ui_background_reg,
        "^\\s*ui_background"
        "\\s+texture=([\\w-. \\(\\)/+-]+)"
        "\\s+update=(once|always)"
        "\\s+focus_mask=(menu|loading|countdown_any|scene)$");
    static const DECLARE_REGEX(hud_image_reg,
        "^\\s*hud_image"
        "\\s+name=([\\w+-.]+)"
        "\\s+camera_node=([\\w+-.]+)"
        "\\s+filename=([\\w+-. \\(\\)/\\\\:]+)"
        "\\s+update=(once|always)"
        "\\s+center=([\\w+-.]+)\\s+([\\w+-.]+)"
        "\\s+size=([\\w+-.]+)\\s+([\\w+-.]+)$");
    static const DECLARE_REGEX(perspective_camera_reg,
        "^\\s*perspective_camera"
        "\\s+node=([\\w+-.]+)"
        "\\s+y_fov=([\\w+-.]+)"
        "\\s+near_plane=([\\w+-.]+)"
        "\\s+far_plane=([\\w+-.]+)"
        "\\s+requires_postprocessing=(0|1)$");
    static const DECLARE_REGEX(ortho_camera_reg,
        "^\\s*ortho_camera"
        "\\s+node=([\\w+-.]+)"
        "\\s+near_plane=([\\w+-.]+)"
        "\\s+far_plane=([\\w+-.]+)"
        "\\s+left_plane=([\\w+-.]+)"
        "\\s+right_plane=([\\w+-.]+)"
        "\\s+bottom_plane=([\\w+-.]+)"
        "\\s+top_plane=([\\w+-.]+)"
        "\\s+requires_postprocessing=(0|1)$");
    static const DECLARE_REGEX(light_reg,
        "^\\s*light"
        "\\s+node=([\\w+-.]+)"
        "\\s+black_node=([\\w+-.]*)"
        "\\s+update=(once|always)"
        "\\s+with_depth_texture=(0|1)"
        "\\s+ambience=([\\w+-.]+)"
        "\\s+([\\w+-.]+)\\s+([\\w+-.]+)"
        "\\s+diffusivity=([\\w+-.]+)\\s+([\\w+-.]+)\\s+([\\w+-.]+)"
        "\\s+specularity=([\\w+-.]+)\\s+([\\w+-.]+)\\s+([\\w+-.]+)"
        "\\s+shadow=(0|1)$");
    static const DECLARE_REGEX(look_at_node_reg,
        "^\\s*look_at_node"
        "\\s+follower=([\\w+-.]+)"
        "\\s+followed=([\\w+-.]+)$");
    static const DECLARE_REGEX(keep_offset_reg,
        "^\\s*keep_offset"
        "\\s+follower=([\\w+-.]+)"
        "\\s+followed=([\\w+-.]+)"
        "\\s+offset=([\\w+-.]+)\\s+([\\w+-.]+)\\s+([\\w+-.]+)$");
    static const DECLARE_REGEX(follow_node_reg,
        "^\\s*follow_node"
        "\\s+follower=([\\w+-.]+)"
        "\\s+followed=([\\w+-.]+)"
        "\\s+distance=([\\w+-.]+)"
        "\\s+node_displacement=([\\w+-.]+)\\s+([\\w+-.]+)\\s+([\\w+-.]+)"
        "\\s+look_at_displacement=([\\w+-.]+)\\s+([\\w+-.]+)\\s+([\\w+-.]+)"
        "\\s+snappiness=([\\w+-.]+)"
        "\\s+y_adaptivity=([\\w+-.]+)"
        "\\s+y_snappiness=([\\w+-.]+)$");
    static const DECLARE_REGEX(add_audio_reg,
        "^\\s*add_audio"
        "\\s+name=([\\w+-.]+)"
        "\\s+filename=([\\w-. \\(\\)/+-]+)"
        "\\s+gain=([\\w+-.]+)$");
    static const DECLARE_REGEX(add_texture_descriptor_reg,
        "^\\s*add_texture_descriptor"
        "\\s+name=([\\w+-.]+)"
        "\\s+color=(#?[\\w-. \\(\\)/+-]+)"
        "(?:\\s+normal=([#\\w-. \\(\\)/+-]+))?"
        "\\s+color_mode=(grayscale|rgb|rgba)"
        "(?:\\s+desaturate=(0|1))?"
        "(?:\\s+histogram=([#\\w-. \\(\\)/+-]+))?"
        "(?:\\s+mixed=([#\\w-. \\(\\)/+-]+))?"
        "(?:\\s+overlap_npixels=(\\d+))?"
        "(?:\\s+mean_color=([\\w+-.]+)\\s+([\\w+-.]+)\\s+([\\w+-.]+))?"
        "(?:\\s+lighten=([\\w+-.]+)\\s+([\\w+-.]+)\\s+([\\w+-.]+))?"
        "\\s+anisotropic_filtering_level=(\\d+)$");
    static const DECLARE_REGEX(add_blend_map_texture_reg,
        "^\\s*add_blend_map_texture"
        "\\s+name=([\\w-. \\(\\)/+-]+)"
        "\\s+texture=(#?[\\w-.\\(\\)/+-]+)"
        "\\s+min_height=([\\w+-.]+)"
        "\\s+max_height=([\\w+-.]+)"
        "\\s+distances=([\\w+-.]+)\\s+([\\w+-.]+)\\s+([\\w+-.]+)\\s+([\\w+-.]+)"
        "\\s+normal=([\\w+-.]+)\\s+([\\w+-.]+)\\s+([\\w+-.]+)"
        "\\s+cosine=([\\w+-.]+)\\s+([\\w+-.]+)\\s+([\\w+-.]+)\\s+([\\w+-.]+)"
        "(?:\\s+discreteness=([\\w+-.]+))?"
        "\\s+scale=([\\w+-.]+)"
        "\\s+weight=([\\w+-.]+)$");
    static const DECLARE_REGEX(add_texture_atlas_reg,
        "^\\s*add_texture_atlas"
        "\\s+name=([\\w+-.]+)"
        "\\s+width=(\\d+)"
        "\\s+height=(\\d+)"
        "\\s+color_mode=(grayscale|rgb|rgba)"
        "\\s+images=([\\s\\S]*)$");
    static const DECLARE_REGEX(save_texture_atlas_png_reg,
        "^\\s*save_texture_atlas_png"
        "\\s+name=([\\w+-.]+)"
        "\\s+filename=([\\w+-. \\(\\)/\\\\:]+)"
        "\\s+color_mode=(grayscale|rgb|rgba)");
    static const DECLARE_REGEX(record_track_reg,
        "^\\s*record_track"
        "\\s+node=([\\w+-.]+)"
        "\\s+filename=([\\w+-. \\(\\)/\\\\:]+)$");
    static const DECLARE_REGEX(record_track_gpx_reg,
        "^\\s*record_track_gpx"
        "\\s+node=([\\w+-.]+)"
        "\\s+filename=([\\w+-. \\(\\)/\\\\:]+)$");
    static const DECLARE_REGEX(playback_track_reg,
        "^\\s*playback_track"
        "\\s+node=([\\w+-.]+)"
        "\\s+speed=([\\w+-.]+)"
        "\\s+filename=([\\w+-. \\(\\)/\\\\:]+)$");
    static const DECLARE_REGEX(define_winner_conditionals_reg,
        "^\\s*define_winner_conditionals"
        "\\s+begin_rank=(\\d+)"
        "\\s+end_rank=(\\d+)$");
    static const DECLARE_REGEX(playback_winner_track_reg,
        "^\\s*playback_winner_track"
        "\\s+node=([\\w+-.]+)"
        "\\s+speed=([\\w+-.]+)"
        "\\s+rank=(\\d+)$");
    static const DECLARE_REGEX(check_points_reg,
        "^\\s*check_points"
        "\\s+moving_node=([\\w+-.]+)"
        "\\s+resource=([\\w-. \\(\\)/+-]+)"
        "\\s+player=([\\w+-.]+)"
        "\\s+nbeacons=(\\d+)"
        "\\s+nth=(\\d+)"
        "\\s+nahead=(\\d+)"
        "\\s+radius=([\\w+-.]+)"
        "\\s+height_changed=(0|1)"
        "\\s+track_filename=([\\w+-. \\(\\)/\\\\:]+)"
        "\\s+on_finish=([\\w+-.:= ]*)$");
    static const DECLARE_REGEX(set_camera_cycle_reg, "^\\s*set_camera_cycle name=(near|far)((?: [\\w+-.]+)*)$");
    static const DECLARE_REGEX(set_camera_reg, "^\\s*set_camera ([\\w+-.]+)$");
    static const DECLARE_REGEX(set_dirtmap_reg,
        "^\\s*set_dirtmap"
        "\\s+filename=([\\w+-. \\(\\)/\\\\:]+)"
        "\\s+offset=([\\w+-.]+)"
        "\\s+discreteness=([\\w+-.]+)"
        "\\s+scale=([\\w+-.]+)"
        "\\s+wrap_mode=(repeat|clamp_to_edge|clamp_to_border)$");
    static const DECLARE_REGEX(set_soft_light_reg,
        "^\\s*set_soft_light"
        "\\s+filename=([\\w+-. \\(\\)/\\\\:]*)$");
    static const DECLARE_REGEX(set_skybox_reg,
        "^\\s*set_skybox"
        "\\s+alias=([\\w+-.]+)"
        "\\s+filenames=([\\w-. \\(\\)/+-]+) ([\\w-. \\(\\)/+-]+) ([\\w-. \\(\\)/+-]+) ([\\w-. \\(\\)/+-]+) ([\\w-. \\(\\)/+-]+) ([\\w-. \\(\\)/+-]+)$");
    static const DECLARE_REGEX(set_vip_reg, "^\\s*set_vip player=([\\w+-.]+)$");
    static const DECLARE_REGEX(respawn_all_players_reg, "^\\s*respawn_all_players$");
    static const DECLARE_REGEX(burn_in_reg, "^\\s*burn_in seconds=([\\w+-.]+)$");
    static const DECLARE_REGEX(append_focuses_reg,
        "^\\s*append_focuses"
        "((?:\\s+(?:menu|loading|countdown_pending|scene|game_over))+)$");
    static const DECLARE_REGEX(set_focuses_reg,
        "^\\s*set_focuses"
        "((\\s+(?:menu|loading|countdown_pending|scene))+)$");
    static const DECLARE_REGEX(wayside_resource_names_reg,
        "^\\s+min_dist:([\\w+-.]+)"
        "\\s+max_dist:([\\w+-.]+)"
        "([\\s:\\w-. \\(\\)/+-]*)$");
    static const DECLARE_REGEX(set_way_points_reg, "^\\s*"
        "set_way_points"
        "\\s+player=([\\w+-.]+)"
        "\\s+node=([\\w+-.]+)"
        "\\s+resource=([\\w+-.]+)$");
    static const DECLARE_REGEX(set_pod_bot_way_points_reg, "^\\s*"
        "set_pod_bot_way_points"
        "\\s+node=([\\w+-.]+)"
        "\\s+resource=([\\w+-.]+)$");
    static const DECLARE_REGEX(set_pod_bot_game_mode_reg, "^\\s*"
        "set_pod_bot_game_mode"
        "\\s+(as|cs|de|awp|aim|fy|es)$");
    static const DECLARE_REGEX(set_can_drive_reg, "^\\s*"
        "set_can_drive"
        "\\s+player=([\\w+-.]+)"
        "\\s+value=(0|1)$");
    static const DECLARE_REGEX(set_can_aim_reg, "^\\s*"
        "set_can_aim"
        "\\s+player=([\\w+-.]+)"
        "\\s+value=(0|1)$");
    static const DECLARE_REGEX(set_can_shoot_reg, "^\\s*"
        "set_can_shoot"
        "\\s+player=([\\w+-.]+)"
        "\\s+value=(0|1)$");
    static const DECLARE_REGEX(pause_on_lose_focus_reg,
        "^\\s*pause_on_lose_focus"
        "\\s+focus_mask=(menu|loading|countdown_any|scene)"
        "\\s+submenu=(\\w*)$");
    static const DECLARE_REGEX(repeat_reg,
        "^\\s*repeat"
        "\\s+([\\s\\S]+)$");
    static const DECLARE_REGEX(add_node_not_allowed_to_be_unregistered_reg,
        "^\\s*add_node_not_allowed_to_be_unregistered"
        "\\s+name=([\\w+-.]+)$");
    static const DECLARE_REGEX(remove_node_not_allowed_to_be_unregistered_reg,
        "^\\s*remove_node_not_allowed_to_be_unregistered"
        "\\s+name=([\\w+-.]+)$");
    static const DECLARE_REGEX(clear_nodes_not_allowed_to_be_unregistered_reg,
        "^\\s*clear_nodes_not_allowed_to_be_unregistered$");
    static const DECLARE_REGEX(execute_in_physics_thread_reg,
        "^\\s*execute_in_physics_thread"
        "\\s+([\\s\\S]+)$");

    MacroLineExecutor::UserFunction user_function = [&](
        const std::string& context,
        const std::function<FPath(const std::string&)>& fpath,
        const MacroLineExecutor& macro_line_executor,
        const std::string& line,
        SubstitutionMap* local_substitutions) -> bool
    {
        {
            auto renderable_scene = [&]() -> RenderableScene& {
                auto cit = renderable_scenes.find(context);
                if (cit == renderable_scenes.end()) {
                    throw std::runtime_error("Could not find renderable scene with name \"" + context + '"');
                }
                return *cit->second;
            };
            UserFunctionArgs args{
                .line = line,
                .renderable_scene = renderable_scene,
                .fpath = fpath,
                .macro_line_executor = macro_line_executor,
                .external_substitutions = external_substitutions,
                .local_substitutions = local_substitutions,
                .rsc = rsc,
                .ui_focus = ui_focus,
                .num_renderings = num_renderings,
                .script_filename = script_filename,
                .next_scene_filename = next_scene_filename};
            for (const auto& f : user_functions_) {
                if (f(args))
                {
                    return true;
                }
            }
        }
        auto fpathp = [&](const std::string& v){return fpath(v).path;};
        Mlib::re::smatch match;
        if (Mlib::re::regex_match(line, match, create_scene_reg)) {
            RenderingContextGuard rrg{
                scene_node_resources,
                match[1].str() + ".rendering_resources",
                scene_config.render_config.anisotropic_filtering_level,
                safe_stoi(match[2].str())};
            AggregateRendererGuard arg{std::make_shared<AggregateArrayRenderer>()};
            InstancesRendererGuard irg{std::make_shared<ArrayInstancesRenderer>()};
            auto rs = std::make_shared<RenderableScene>(
                scene_node_resources,
                scene_config,
                button_states,
                cursor_states,
                scroll_wheel_states,
                ui_focus,
                window,
                SceneConfigResource{
                    .fly = safe_stob(match[3].str()),
                    .rotate = safe_stob(match[4].str()),
                    .print_gamepad_buttons = safe_stob(match[5].str()),
                    .depth_fog = safe_stob(match[6].str()),
                    .low_pass = safe_stob(match[7].str()),
                    .high_pass = safe_stob(match[8].str()),
                    .vfx = safe_stob(match[9].str()),
                    .with_dirtmap = safe_stob(match[10].str()),
                    .with_skybox = safe_stob(match[11].str()),
                    .with_flying_logic = safe_stob(match[12].str()),
                    .with_pod_bot = safe_stob(match[13].str()),
                    .clear_mode = clear_mode_from_string(match[14].str())},
                script_filename,
                safe_stoz(match[15].str()));
            if (!renderable_scenes.insert({match[1].str(), rs}).second) {
                throw std::runtime_error("Scene with name \"" + match[1].str() + "\" already exists");
            }
            return true;
        }
        if (Mlib::re::regex_match(line, match, obj_resource_reg)) {
            LoadMeshConfig load_mesh_config{
                .position = FixedArray<float, 3>{
                    safe_stof(match[3].str()),
                    safe_stof(match[4].str()),
                    safe_stof(match[5].str())},
                .rotation = FixedArray<float, 3>{
                    safe_stof(match[6].str()) / 180.f * float(M_PI),
                    safe_stof(match[7].str()) / 180.f * float(M_PI),
                    safe_stof(match[8].str()) / 180.f * float(M_PI)},
                .scale = FixedArray<float, 3>{
                    safe_stof(match[9].str()),
                    safe_stof(match[10].str()),
                    safe_stof(match[11].str())},
                .distances = OrderableFixedArray<float, 2>{
                    match[12].matched ? safe_stof(match[12].str()) : 0.f,
                    match[13].matched ? safe_stof(match[13].str()) : float { INFINITY }},
                .is_small = safe_stob(match[14].str()),
                .blend_mode = blend_mode_from_string(match[15].str()),
                .alpha_distances = {
                    safe_stof(match[16].str()),
                    safe_stof(match[17].str()),
                    safe_stof(match[18].str()),
                    safe_stof(match[19].str())},
                .cull_faces = safe_stob(match[20].str()),
                .occluded_type = occluded_type_from_string(match[21].str()),
                .occluder_type = occluder_type_from_string(match[22].str()),
                .occluded_by_black = safe_stob(match[23].str()),
                .is_black = match[24].matched
                    ? safe_stob(match[24].str())
                    : false,
                .aggregate_mode = aggregate_mode_from_string(match[25].str()),
                .transformation_mode = transformation_mode_from_string(match[26].str()),
                .triangle_tangent_error_behavior = match[27].matched
                    ? triangle_tangent_error_behavior_from_string(match[27].str())
                    : TriangleTangentErrorBehavior::RAISE,
                .apply_static_lighting = false,
                .werror = !match[28].matched};
            std::string filename = fpathp(match[2].str());
            if (filename.ends_with(".obj")) {
                scene_node_resources.add_resource(match[1].str(), std::make_shared<ObjFileResource>(
                    filename,
                    load_mesh_config));
            } else if (filename.ends_with(".mhx2")) {
                scene_node_resources.add_resource(match[1].str(), std::make_shared<Mhx2FileResource>(
                    filename,
                    load_mesh_config));
            } else {
                throw std::runtime_error("Unknown file type: " + filename);
            }
            return true;
        }
        if (Mlib::re::regex_match(line, match, gen_triangle_rays_reg)) {
            scene_node_resources.generate_triangle_rays(
                match[1].str(),
                safe_stoi(match[2].str()),
                {
                    safe_stof(match[3].str()),
                    safe_stof(match[4].str()),
                    safe_stof(match[5].str())
                },
                safe_stoi(match[6].str()));
            return true;
        }
        if (Mlib::re::regex_match(line, match, osm_resource_reg)) {
            OsmResourceConfig config;
            static const DECLARE_REGEX(key_value_reg, "(?:\\s*([^=]+)=([^,]*),?|([\\s\\S]+))");
            std::string resource_name;
            std::vector<float> layer_heights_layer;
            std::vector<float> layer_heights_height;
            find_all(match[1].str(), key_value_reg, [&](const Mlib::re::smatch& match2) {
                if (match2[3].matched) {
                    throw std::runtime_error("Unknown element: \"" + match2[3].str() + '"');
                }
                std::string key = match2[1].str();
                std::string value = rtrim_copy(match2[2].str());
                auto add_street_textures = [&value, &fpathp, &config](RoadType road_type){
                    static const DECLARE_REGEX(
                        street_texture_reg,
                        "(?:\\s*lanes:(\\d+) "
                        "texture0:(#?[\\w-.\\(\\)/+-]+)"
                        "(?: texture1:(#?[\\w-.\\(\\)/+-]+))?"
                        "(?: uvx:([\\w+-.]+))?|"
                        "([\\s\\S]+))");
                    find_all(value, street_texture_reg, [&](const Mlib::re::smatch& match3) {
                        if (match3[5].matched) {
                            throw std::runtime_error("Unknown element: \"" + match3[5].str() + '"');
                        }
                        RoadProperties rp{.type=road_type, .nlanes = safe_stoz(match3[1].str())};
                        std::vector<std::string> textures =
                            match3[3].matched
                                ? std::vector<std::string>{ fpathp(match3[2].str()), fpathp(match3[3].str()) }
                                : std::vector<std::string>{ fpathp(match3[2].str()) };
                        RoadStyle rs{
                            .textures = textures,
                            .uvx = match3[4].matched
                                ? safe_stof(match3[4].str())
                                : 1.f};
                        config.street_texture[rp] = rs;
                    });
                };
                auto add_barrier_textures = [&value, &fpathp, &config](){
                    static const DECLARE_REGEX(
                        barrier_texture_reg,
                        "(?:\\s*name:(\\w+) "
                        "texture:(#?[\\w-.\\(\\)/+-]+) "
                        "uv:([\\w+-.]+)\\s+([\\w+-.]+) "
                        "blend_mode:(off|binary|semi_continuous|continuous) "
                        "wrap_mode_t:(repeat|clamp_to_edge|clamp_to_border) "
                        "reorient_uv0:(0|1)|([\\s\\S]+))");
                    find_all(value, barrier_texture_reg, [&](const Mlib::re::smatch& match3) {
                        if (match3[8].matched) {
                            throw std::runtime_error("Unknown element: \"" + match3[8].str() + '"');
                        }
                        BarrierStyle as{
                            .texture = fpathp(match3[2].str()),
                            .uv = FixedArray<float, 2>{
                                safe_stof(match3[3].str()),
                                safe_stof(match3[4].str())},
                            .blend_mode = blend_mode_from_string(match3[5].str()),
                            .wrap_mode_t = wrap_mode_from_string(match3[6].str()),
                            .reorient_uv0 = safe_stob(match3[7].str())};
                        if (!config.barrier_textures.insert({match3[1].str(), as}).second) {
                            throw std::runtime_error("Duplicate barrier style");
                        }
                    });
                };
                if (key.starts_with("#")) {
                    // Do nothing
                } else if (key == "name") {
                    resource_name = value;
                }
                else if (key == "filename") {
                    config.filename = fpathp(value);
                }
                else if (key == "heightmap") {
                    config.heightmap = fpathp(value);
                }
                else if (key == "heightmap_mask") {
                    config.heightmap_mask = fpathp(value);
                }
                else if (key == "heightmap_extension") {
                    config.heightmap_extension = safe_stoz(value);
                }
                else if (key == "terrain_undefined_textures") {
                    config.terrain_textures[TerrainType::UNDEFINED] = string_to_vector(value, fpathp);
                }
                else if (key == "terrain_grass_textures") {
                    config.terrain_textures[TerrainType::GRASS] = string_to_vector(value, fpathp);
                }
                else if (key == "terrain_flowers_textures") {
                    config.terrain_textures[TerrainType::FLOWERS] = string_to_vector(value, fpathp);
                }
                else if (key == "terrain_elevated_grass_textures") {
                    config.terrain_textures[TerrainType::ELEVATED_GRASS] = string_to_vector(value, fpathp);
                }
                else if (key == "terrain_elevated_grass_base_textures") {
                    config.terrain_textures[TerrainType::ELEVATED_GRASS_BASE] = string_to_vector(value, fpathp);
                }
                else if (key == "terrain_stone_textures") {
                    config.terrain_textures[TerrainType::STONE] = string_to_vector(value, fpathp);
                }
                else if (key == "terrain_asphalt_textures") {
                    config.terrain_textures[TerrainType::ASPHALT] = string_to_vector(value, fpathp);
                }
                else if (key == "terrain_water_floor_textures") {
                    config.terrain_textures[TerrainType::WATER_FLOOR] = string_to_vector(value, fpathp);
                }
                else if (key == "terrain_water_floor_base_textures") {
                    config.terrain_textures[TerrainType::WATER_FLOOR_BASE] = string_to_vector(value, fpathp);
                }
                else if (key == "stone_dirt_texture") {
                    config.terrain_dirt_textures[TerrainType::STONE] = fpathp(value);
                }
                else if (key == "grass_dirt_texture") {
                    config.terrain_dirt_textures[TerrainType::GRASS] = fpathp(value);
                }
                else if (key == "street_dirt_texture") {
                    config.street_dirt_texture = fpathp(value);
                }
                else if (key == "street_crossing_texture") {
                    config.street_crossing_texture[RoadType::STREET] = fpathp(value);
                }
                else if (key == "street_texture") {
                    RoadProperties rp{.type=RoadType::STREET, .nlanes = 1};
                    RoadStyle rs{.textures = { fpathp(value) }, .uvx = 1.f};
                    config.street_texture[rp] = rs;
                }
                else if (key == "street_textures") {
                    add_street_textures(RoadType::STREET);
                }
                else if (key == "path_crossing_texture") {
                    config.street_crossing_texture[RoadType::PATH] = fpathp(value);
                }
                else if (key == "path_texture") {
                    RoadProperties rp{.type=RoadType::PATH, .nlanes = 1};
                    RoadStyle rs{.textures = { fpathp(value) }, .uvx = 1.f};
                    config.street_texture[rp] = rs;
                }
                else if (key == "path_textures") {
                    add_street_textures(RoadType::PATH);
                }
                else if (key == "wall_textures") {
                    add_street_textures(RoadType::WALL);
                }
                else if (key == "curb_street_texture") {
                    config.curb_street_texture[RoadType::STREET] = fpathp(value);
                }
                else if (key == "curb_path_texture") {
                    config.curb_street_texture[RoadType::PATH] = fpathp(value);
                }
                else if (key == "curb_wall_texture") {
                    config.curb_street_texture[RoadType::WALL] = fpathp(value);
                }
                else if (key == "curb2_street_texture") {
                    config.curb2_street_texture[RoadType::STREET] = fpathp(value);
                }
                else if (key == "curb2_path_texture") {
                    config.curb2_street_texture[RoadType::PATH] = fpathp(value);
                }
                else if (key == "curb2_wall_texture") {
                    config.curb2_street_texture[RoadType::WALL] = fpathp(value);
                }
                else if (key == "air_curb_street_texture") {
                    config.air_curb_street_texture[RoadType::STREET] = fpathp(value);
                }
                else if (key == "air_curb_path_texture") {
                    config.air_curb_street_texture[RoadType::PATH] = fpathp(value);
                }
                else if (key == "air_support_texture") {
                    config.air_support_texture = fpathp(value);
                }
                else if (key == "socle_textures") {
                    config.socle_textures = string_to_vector(value, fpathp);
                }
                else if (key == "facade_textures") {
                    config.facade_textures = string_to_vector(value, parse_facade_texture);
                }
                else if (key == "ceiling_texture") {
                    config.ceiling_texture = fpathp(value);
                }
                else if (key == "barrier_textures") {
                    add_barrier_textures();
                }
                else if (key == "roof_texture") {
                    config.roof_texture = fpathp(value);
                }
                else if (key == "tunnel_pipe_texture") {
                    config.tunnel_pipe_texture = fpathp(value);
                }
                else if (key == "tunnel_pipe_resource_name") {
                    config.tunnel_pipe_resource_name = value;
                }
                else if (key == "tunnel_bdry_resource_name") {
                    config.tunnel_bdry_resource_name = value;
                }
                else if (key == "street_surface_central_resource_name") {
                    config.street_surface_central_resource_names[RoadType::STREET] = value;
                }
                else if (key == "street_surface_endpoint0_resource_name") {
                    config.street_surface_endpoint0_resource_names[RoadType::STREET] = value;
                }
                else if (key == "street_surface_endpoint1_resource_name") {
                    config.street_surface_endpoint1_resource_names[RoadType::STREET] = value;
                }
                else if (key == "path_surface_central_resource_name") {
                    config.street_surface_central_resource_names[RoadType::PATH] = value;
                }
                else if (key == "path_surface_endpoint0_resource_name") {
                    config.street_surface_endpoint0_resource_names[RoadType::PATH] = value;
                }
                else if (key == "path_surface_endpoint1_resource_name") {
                    config.street_surface_endpoint1_resource_names[RoadType::PATH] = value;
                }
                else if (key == "water_texture") {
                    config.water_texture = fpathp(value);
                }
                else if (key == "water_height") {
                    config.water_height = safe_stof(value);
                }
                else if (key == "tree_resource_names") {
                    config.tree_resource_names = string_to_vector(value, [&scene_node_resources](const std::string& name){return parse_resource_name(scene_node_resources, name);});
                }
                else if (key == "grass_resource_names") {
                    config.grass_resource_names = string_to_vector(value, [&scene_node_resources](const std::string& name){return parse_resource_name(scene_node_resources, name);});
                }
                else if (key == "near_grass_resource_names") {
                    config.near_grass_terrain_style.near_resource_names = string_to_vector(value, [&scene_node_resources](const std::string& name){return parse_resource_name(scene_node_resources, name);});
                }
                else if (key == "near_flowers_resource_names") {
                    config.near_flowers_terrain_style.near_resource_names = string_to_vector(value, [&scene_node_resources](const std::string& name){return parse_resource_name(scene_node_resources, name);});
                }
                else if (key == "dirt_decals_resource_names") {
                    config.dirt_decals_terrain_style.near_resource_names = string_to_vector(value, [&scene_node_resources](const std::string& name){return parse_resource_name(scene_node_resources, name);});
                }
                else if (key == "wayside_resource_names") {
                    Mlib::re::smatch match2;
                    if (!Mlib::re::regex_match(value, match2, wayside_resource_names_reg)) {
                        throw std::runtime_error("Could not parse \"" + value + '"');
                    }
                    config.waysides.push_back(WaysideResourceNames{
                        .min_dist = safe_stof(match2[1].str()),
                        .max_dist = safe_stof(match2[2].str()),
                        .resource_names = string_to_vector(match2[3].str(), [&scene_node_resources](const std::string& name){return parse_resource_name(scene_node_resources, name);}) });
                }
                else if (key == "bounding_terrain_type") {
                    config.bounding_terrain_type = terrain_type_from_string(value);
                }
                else if (key == "default_terrain_type") {
                    config.default_terrain_type = terrain_type_from_string(value);
                }
                else if (key == "default_street_width") {
                    config.default_street_width = safe_stof(value);
                }
                else if (key == "default_lane_width") {
                    config.default_lane_width = safe_stof(value);
                }
                else if (key == "default_tunnel_pipe_width") {
                    config.default_tunnel_pipe_width = safe_stof(value);
                }
                else if (key == "default_tunnel_pipe_height") {
                    config.default_tunnel_pipe_height = safe_stof(value);
                }
                else if (key == "roof_width") {
                    config.roof_width = safe_stof(value);
                }
                else if (key == "scale") {
                    config.scale = safe_stof(value);
                }
                else if (key == "uv_scale_terrain") {
                    config.uv_scale_terrain = safe_stof(value);
                }
                else if (key == "uv_scale_street") {
                    config.uv_scales_street[RoadType::STREET] = safe_stof(value);
                }
                else if (key == "uv_scale_path") {
                    config.uv_scales_street[RoadType::PATH] = safe_stof(value);
                }
                else if (key == "uv_scale_wall") {
                    config.uv_scales_street[RoadType::WALL] = safe_stof(value);
                }
                else if (key == "uv_scale_facade") {
                    config.uv_scale_facade = safe_stof(value);
                }
                else if (key == "uv_scale_ceiling") {
                    config.uv_scale_ceiling = safe_stof(value);
                }
                else if (key == "uv_scale_barrier_wall") {
                    config.uv_scale_barrier_wall = safe_stof(value);
                }
                else if (key == "uv_scale_highway_wall") {
                    config.uv_scale_highway_wall = safe_stof(value);
                }
                else if (key == "uv_scale_crossings") {
                    config.uv_scale_crossings = safe_stof(value);
                }
                else if (key == "with_roofs") {
                    config.with_roofs = safe_stob(value);
                }
                else if (key == "with_ceilings") {
                    config.with_ceilings = safe_stob(value);
                }
                else if (key == "building_bottom") {
                    config.building_bottom = safe_stof(value);
                }
                else if (key == "default_building_top") {
                    config.default_building_top = safe_stof(value);
                }
                else if (key == "default_barrier_top") {
                    config.default_barrier_top = safe_stof(value);
                }
                else if (key == "remove_backfacing_triangles") {
                    config.remove_backfacing_triangles = safe_stob(value);
                }
                else if (key == "with_tree_nodes") {
                    config.with_tree_nodes = safe_stob(value);
                }
                else if (key == "forest_outline_tree_distance") {
                    config.forest_outline_tree_distance = safe_stof(value);
                }
                else if (key == "forest_outline_tree_inwards_distance") {
                    config.forest_outline_tree_inwards_distance = safe_stof(value);
                }
                else if (key == "much_grass_distance") {
                    config.much_grass_distance = safe_stof(value);
                }
                else if (key == "much_near_grass_distance") {
                    config.near_grass_terrain_style.much_near_distance = safe_stof(value);
                }
                else if (key == "much_near_flowers_distance") {
                    config.near_flowers_terrain_style.much_near_distance = safe_stof(value);
                }
                else if (key == "dirt_decals_distance") {
                    config.dirt_decals_terrain_style.much_near_distance = safe_stof(value);
                }
                else if (key == "much_near_grass_distance_to_bdry") {
                    config.near_grass_terrain_style.min_near_distance_to_bdry = safe_stof(value);
                }
                else if (key == "raceway_beacon_distance") {
                    config.raceway_beacon_distance = safe_stof(value);
                }
                else if (key == "with_terrain") {
                    config.with_terrain = safe_stob(value);
                }
                else if (key == "with_buildings") {
                    config.with_buildings = safe_stob(value);
                }
                else if (key == "only_raceways_and_walls") {
                    config.only_raceways_and_walls = safe_stob(value);
                }
                else if (key == "highway_name_pattern") {
                    config.highway_name_pattern = value;
                }
                else if (key == "excluded_highways") {
                    config.excluded_highways = string_to_set(value);
                }
                else if (key == "path_tags") {
                    config.path_tags = string_to_set(value);
                }
                else if (key == "steiner_point_distances_road") {
                    config.steiner_point_distances_road = string_to_vector(value, safe_stof);
                }
                else if (key == "steiner_point_distances_steiner") {
                    config.steiner_point_distances_steiner = string_to_vector(value, safe_stof);
                }
                else if (key == "curb_alpha") {
                    config.curb_alpha = safe_stof(value);
                }
                else if (key == "curb2_alpha") {
                    config.curb2_alpha = safe_stof(value);
                }
                else if (key == "curb_uv_x") {
                    config.curb_uv(0) = safe_stof(value);
                }
                else if (key == "curb_uv_y") {
                    config.curb_uv(1) = safe_stof(value);
                }
                else if (key == "curb2_uv_x") {
                    config.curb2_uv(0) = safe_stof(value);
                }
                else if (key == "curb2_uv_y") {
                    config.curb2_uv(1) = safe_stof(value);
                }
                else if (key == "curb_color") {
                    auto curb_color = string_to_vector(value, safe_stof, 3);
                    config.curb_color = FixedArray<float, 3>{ curb_color[0], curb_color[1], curb_color[2] };
                }
                else if (key == "raise_streets_amount") {
                    config.raise_streets_amount = safe_stof(value);
                }
                else if (key == "extrude_curb_amount") {
                    config.extrude_curb_amount = safe_stof(value);
                }
                else if (key == "extrude_street_amount") {
                    config.extrude_street_amount = safe_stof(value);
                }
                else if (key == "extrude_air_curb_amount") {
                    config.extrude_air_curb_amount = safe_stof(value);
                }
                else if (key == "extrude_air_support_amount") {
                    config.extrude_air_support_amount = safe_stof(value);
                }
                else if (key == "extrude_wall_amount") {
                    config.extrude_wall_amount = safe_stof(value);
                }
                else if (key == "extrude_grass_amount") {
                    config.extrude_grass_amount = safe_stof(value);
                }
                else if (key == "extrude_elevated_grass_amount") {
                    config.extrude_elevated_grass_amount = safe_stof(value);
                }
                else if (key == "extrude_water_floor_amout") {
                    config.extrude_water_floor_amout = safe_stof(value);
                }
                else if (key == "street_light_resource_names") {
                    config.street_light_resource_names = string_to_vector(value, [&scene_node_resources](const std::string& name){return parse_resource_name(scene_node_resources, name);});
                }
                else if (key == "max_wall_width") {
                    config.max_wall_width = safe_stof(value);
                }
                else if (key == "with_height_bindings") {
                    config.with_height_bindings = safe_stob(value);
                }
                else if (key == "street_node_smoothness") {
                    config.street_node_smoothness = safe_stof(value);
                }
                else if (key == "street_edge_smoothness") {
                    config.street_edge_smoothness = safe_stof(value);
                }
                else if (key == "terrain_edge_smoothness") {
                    config.terrain_edge_smoothness = safe_stof(value);
                }
                else if (key == "driving_direction") {
                    config.driving_direction = driving_direction_from_string(value);
                }
                else if (key == "blend_street") {
                    config.blend_street = safe_stob(value);
                }
                else if (key == "layer_heights_layer") {
                    layer_heights_layer = string_to_vector(value, safe_stof);
                }
                else if (key == "layer_heights_height") {
                    layer_heights_height = string_to_vector(value, safe_stof);
                }
                else if (key == "game_level") {
                    config.game_level = value;
                }
                else if (key == "base_osm_map_resource") {
                    config.base_osm_map_resource = value;
                }
                else {
                    throw std::runtime_error("Unknown osm key: \"" + key + '"');
                }
            });
            if (resource_name.empty()) {
                throw std::runtime_error("Osm resource name not set");
            }
            config.layer_heights = Interp<float>(
                layer_heights_layer,
                layer_heights_height);
            std::string cache_filename = fpathp(config.filename + '_' + config.game_level + ".cereal.binary");
            std::shared_ptr<OsmMapResource> osm_map_resource;
            bool enable_cache = getenv_default_bool("ENABLE_OSM_MAP_CACHE", true);
            if (enable_cache && fs::exists(cache_filename)) {
                osm_map_resource = std::make_shared<OsmMapResource>(
                    scene_node_resources,
                    cache_filename);
            } else {
                osm_map_resource = std::make_shared<OsmMapResource>(
                    scene_node_resources,
                    config);
                if (enable_cache) {
                    osm_map_resource->save_to_file(cache_filename);
                }
            }
            scene_node_resources.add_resource(resource_name, osm_map_resource);
            return true;
        }
        if (Mlib::re::regex_match(line, match, gen_ray_reg)) {
            scene_node_resources.generate_ray(
                match[1].str(),
                FixedArray<float, 3>{
                    safe_stof(match[2].str()),
                    safe_stof(match[3].str()),
                    safe_stof(match[4].str())},
                FixedArray<float, 3>{
                    safe_stof(match[5].str()),
                    safe_stof(match[6].str()),
                    safe_stof(match[7].str())});
            return true;
        }
        if (Mlib::re::regex_match(line, match, downsample_reg)) {
            scene_node_resources.downsample(
                match[1].str(),
                safe_stoz(match[2].str()));
            return true;
        }
        if (Mlib::re::regex_match(line, match, import_bone_weights_reg)) {
            scene_node_resources.import_bone_weights(
                match[1].str(),
                match[2].str(),
                safe_stof(match[3].str()));
            return true;
        }
        if (Mlib::re::regex_match(line, match, square_resource_reg)) {
            // 1: name
            // 2: texture_filename
            // 3, 4: min
            // 5, 6: max
            // 7, 8: distances
            // 9: is_small
            // 10: occluded_type
            // 11: occluder_type
            // 12: occluded_by_black
            // 13, 14, 15: ambience
            // 16: blend_mode
            // 17: depth_func
            // 18, 19, 20, 21: alpha_distances
            // 22: cull_faces
            // 23, 24, 25: rotation
            // 26, 27, 28: translation
            // 29: aggregate_mode
            // 30: transformation_mode
            // 31: number_of_frames
            // 32: billboards
            std::vector<BillboardAtlasInstance> billboard_atlas_instances;
            static const DECLARE_REGEX(
                street_texture_reg,
                "(?:\\s*uv_scale:\\s*([\\w+-.]+)\\s+([\\w+-.]+)"
                "\\s+uv_offset:\\s*([\\w+-.]+)\\s+([\\w+-.]+)"
                "\\s+vertex_scale:\\s*([\\w+-.]+)\\s+([\\w+-.]+)"
                "\\s+is_small:(0|1)"
                "|([\\s\\S]+))");
            find_all(match[32].str(), street_texture_reg, [&](const Mlib::re::smatch& match3) {
                if (match3[8].matched) {
                    throw std::runtime_error("Unknown element: \"" + match3[8].str() + '"');
                }
                billboard_atlas_instances.push_back(BillboardAtlasInstance{
                    .uv_scale = OrderableFixedArray<float, 2>{safe_stof(match3[1].str()), safe_stof(match3[2].str())},
                    .uv_offset = OrderableFixedArray<float, 2>{safe_stof(match3[3].str()), safe_stof(match3[4].str())},
                    .vertex_scale = OrderableFixedArray<float, 2>{safe_stof(match3[5].str()), safe_stof(match3[6].str())},
                    .is_small = safe_stob(match3[7].str())
                });
            });
            scene_node_resources.add_resource(match[1].str(), std::make_shared<SquareResource>(
                FixedArray<float, 2, 2>{
                    safe_stof(match[3].str()), safe_stof(match[4].str()),
                    safe_stof(match[5].str()), safe_stof(match[6].str())},
                TransformationMatrix<float, 3>(
                    tait_bryan_angles_2_matrix(
                        FixedArray<float, 3>{
                            match[23].matched ? safe_stof(match[23].str()) / 180.f * float(M_PI) : 0.f,
                            match[24].matched ? safe_stof(match[24].str()) / 180.f * float(M_PI) : 0.f,
                            match[25].matched ? safe_stof(match[25].str()) / 180.f * float(M_PI) : 0.f}),
                    FixedArray<float, 3>{
                        match[26].matched ? safe_stof(match[26].str()) : 0.f,
                        match[27].matched ? safe_stof(match[27].str()) : 0.f,
                        match[28].matched ? safe_stof(match[28].str()) : 0.f}),
                Material{
                    .blend_mode = blend_mode_from_string(match[16].str()),
                    .depth_func = match[17].matched ? depth_func_from_string(match[17].str()) : DepthFunc::LESS,
                    .textures = {{.texture_descriptor = {.color = fpathp(match[2].str())}}},
                    .occluded_type =  occluded_type_from_string(match[10].str()),
                    .occluder_type = occluder_type_from_string(match[11].str()),
                    .occluded_by_black = safe_stob(match[12].str()),
                    .alpha_distances = {
                        safe_stof(match[18].str()),
                        safe_stof(match[19].str()),
                        safe_stof(match[20].str()),
                        safe_stof(match[21].str())},
                    .wrap_mode_s = WrapMode::CLAMP_TO_EDGE,
                    .wrap_mode_t = WrapMode::CLAMP_TO_EDGE,
                    .collide = false,
                    .aggregate_mode = aggregate_mode_from_string(match[29].str()),
                    .transformation_mode = transformation_mode_from_string(match[30].str()),
                    .billboard_atlas_instances = billboard_atlas_instances,
                    .number_of_frames = match[31].matched ? safe_stou(match[31].str()) : 1,
                    .distances = OrderableFixedArray<float, 2>{
                        match[7].matched ? safe_stof(match[7].str()) : 0.f,
                        match[8].matched ? safe_stof(match[8].str()) : float { INFINITY }},
                    .is_small = safe_stob(match[9].str()),
                    .cull_faces = safe_stob(match[22].str()),
                    .ambience = {
                        safe_stof(match[13].str()),
                        safe_stof(match[14].str()),
                        safe_stof(match[15].str())},
                    .diffusivity = {0.f, 0.f, 0.f},
                    .specularity = {0.f, 0.f, 0.f}}.compute_color_mode()));
            return true;
        }
        if (Mlib::re::regex_match(line, match, blending_x_resource_reg)) {
            scene_node_resources.add_resource(match[1].str(), std::make_shared<BlendingXResource>(
                FixedArray<float, 2, 2>{
                    safe_stof(match[3].str()), safe_stof(match[4].str()),
                    safe_stof(match[5].str()), safe_stof(match[6].str())},
                fpathp(match[2].str())));
            return true;
        }
        if (Mlib::re::regex_match(line, match, binary_x_resource_reg)) {
            // 1: name
            // 2: texture_filename_0
            // 3: texture_filename_90
            // 4, 5: min
            // 6, 7: max
            // 8, 9: distances
            // 10: is_small
            // 11: occluded_type
            // 12: occluder_type
            // 13: occluded_by_black
            // 14, 15, 16: ambience
            // 17: blend_mode
            // 18: depth_func
            // 19, 20, 21, 22: alpha_distances
            // 23: cull_faces
            // 24, 25, 26: rotation
            // 27, 28, 29: translation
            // 30: aggregate_mode
            // 31: transformation_mode
            Material material{
                .blend_mode = blend_mode_from_string(match[17].str()),
                .occluded_type =  occluded_type_from_string(match[11].str()),
                .occluder_type = occluder_type_from_string(match[12].str()),
                .occluded_by_black = safe_stob(match[13].str()),
                .alpha_distances = {
                    safe_stof(match[18].str()),
                    safe_stof(match[19].str()),
                    safe_stof(match[20].str()),
                    safe_stof(match[21].str())},
                .wrap_mode_s = WrapMode::CLAMP_TO_EDGE,
                .wrap_mode_t = WrapMode::CLAMP_TO_EDGE,
                .collide = false,
                .aggregate_mode = aggregate_mode_from_string(match[23].str()),
                .transformation_mode = transformation_mode_from_string(match[24].str()),
                .distances = OrderableFixedArray<float, 2>{
                    match[8].matched ? safe_stof(match[8].str()) : 0.f,
                    match[9].matched ? safe_stof(match[9].str()) : float { INFINITY }},
                .is_small = safe_stob(match[10].str()),
                .cull_faces = safe_stob(match[22].str()),
                .ambience = {
                    safe_stof(match[14].str()),
                    safe_stof(match[15].str()),
                    safe_stof(match[16].str())},
                .diffusivity = {0.f, 0.f, 0.f},
                .specularity = {0.f, 0.f, 0.f}};
            Material material_0{material};
            Material material_90{material};
            material_0.textures = {{.texture_descriptor = {.color = fpathp(match[2].str())}}};
            material_90.textures = {{.texture_descriptor = {.color = fpathp(match[3].str())}}};
            material_0.compute_color_mode();
            material_90.compute_color_mode();
            scene_node_resources.add_resource(match[1].str(), std::make_shared<BinaryXResource>(
                FixedArray<float, 2, 2>{
                    safe_stof(match[4].str()), safe_stof(match[5].str()),
                    safe_stof(match[6].str()), safe_stof(match[7].str())},
                material_0,
                material_90));
            return true;
        }
        if (Mlib::re::regex_match(line, match, append_focuses_reg)) {
            for (Focus focus : string_to_vector(match[1].str(), focus_from_string)) {
                ui_focus.focuses.push_back(focus);
            }
            return true;
        }
        if (Mlib::re::regex_match(line, match, set_focuses_reg)) {
            ui_focus.focuses.set_focuses(string_to_vector(match[1].str(), focus_from_string));
            return true;
        }
        if (Mlib::re::regex_match(line, match, add_bvh_resource_reg)) {
            BvhConfig cfg = blender_bvh_config;
            cfg.smooth_radius = safe_stoz(match[3].str());
            cfg.smooth_alpha = safe_stof(match[4].str());
            cfg.periodic = safe_stob(match[5].str());
            scene_node_resources.add_bvh_file(
                match[1].str(),
                fpathp(match[2].str()),
                cfg);
            return true;
        }
        if (Mlib::re::regex_match(line, match, add_audio_reg)) {
            AudioResourceContextStack::primary_audio_resources()->add_buffer(
                match[1].str(),
                match[2].str(),
                match[3].matched ? safe_stof(match[3].str()) : 1);
            return true;
        }
        if (Mlib::re::regex_match(line, match, add_texture_descriptor_reg)) {
            RenderingContextStack::primary_rendering_resources()->add_texture_descriptor(
                match[1].str(),
                TextureDescriptor{
                    .color = fpathp(match[2].str()),
                    .normal = fpathp(match[3].str()),
                    .color_mode = color_mode_from_string(match[4].str()),
                    .desaturate = match[5].matched ? safe_stob(match[5].str()) : false,
                    .histogram = fpathp(match[6].str()),
                    .mixed = match[7].str(),
                    .overlap_npixels = match[8].matched ? safe_stoz(match[8].str()) : 0,
                    .mean_color =
                        OrderableFixedArray<float, 3>{
                            match[9].matched ? safe_stof(match[9].str()) : -1.f,
                            match[10].matched ? safe_stof(match[10].str()) : -1.f,
                            match[11].matched ? safe_stof(match[11].str()) : -1.f},
                    .lighten =
                        OrderableFixedArray<float, 3>{
                            match[12].matched ? safe_stof(match[12].str()) : 0.f,
                            match[13].matched ? safe_stof(match[13].str()) : 0.f,
                            match[14].matched ? safe_stof(match[14].str()) : 0.f},
                    .anisotropic_filtering_level = safe_stou(match[15].str())});
            return true;
        }
        if (Mlib::re::regex_match(line, match, add_blend_map_texture_reg)) {
            auto rr = RenderingContextStack::primary_rendering_resources();
            auto tex = fpath(match[2].str());
            rr->set_blend_map_texture(
                match[1].str(),
                BlendMapTexture{
                    .texture_descriptor = tex.is_variable
                        ? RenderingContextStack::primary_rendering_resources()->get_existing_texture_descriptor(tex.path)
                        : TextureDescriptor{ .color = tex.path },
                    .min_height = safe_stof(match[3].str()),
                    .max_height = safe_stof(match[4].str()),
                    .distances = {
                        safe_stof(match[5].str()),
                        safe_stof(match[6].str()),
                        safe_stof(match[7].str()),
                        safe_stof(match[8].str())},
                    .normal = {
                        safe_stof(match[9].str()),
                        safe_stof(match[10].str()),
                        safe_stof(match[11].str())},
                    .cosines = {
                        safe_stof(match[12].str()),
                        safe_stof(match[13].str()),
                        safe_stof(match[14].str()),
                        safe_stof(match[15].str())},
                    .discreteness = match[16].matched ? safe_stof(match[16].str()) : 2,
                    .scale = safe_stof(match[17].str()),
                    .weight = safe_stof(match[18].str()) });
            return true;
        }
        if (Mlib::re::regex_match(line, match, add_texture_atlas_reg)) {
            std::vector<AtlasTileDescriptor> tiles;
            static const DECLARE_REGEX(
                atlas_tile_reg,
                "(?:\\s*texture_pos:\\s*(\\d+)\\s+(\\d+)"
                "\\s+texture:(#?[\\w-.\\(\\)/+-]+)|"
                "([\\s\\S]+))");
            find_all(match[5].str(), atlas_tile_reg, [&](const Mlib::re::smatch& match2) {
                if (match2[4].matched) {
                    throw std::runtime_error("Unknown element: \"" + match2[4].str() + '"');
                }
                tiles.push_back(AtlasTileDescriptor{
                    .left = safe_stoi(match2[1].str()),
                    .bottom = safe_stoi(match2[2].str()),
                    .filename = fpathp(match2[3].str())});
            });
            RenderingContextStack::primary_rendering_resources()->add_texture_atlas(
                match[1].str(),
                TextureAtlasDescriptor{
                    .width = safe_stoi(match[2].str()),
                    .height = safe_stoi(match[3].str()),
                    .color_mode = color_mode_from_string(match[4].str()),
                    .tiles = tiles});
            return true;
        }
        if (Mlib::re::regex_match(line, match, save_texture_atlas_png_reg)) {
            StbInfo img = RenderingContextStack::primary_rendering_resources()->get_texture_data(TextureDescriptor{
                .color = match[1].str(),
                .color_mode = color_mode_from_string(match[3].str())});
            if (!stbi_write_png(
                match[2].str().c_str(),
                img.width,
                img.height,
                img.nrChannels,
                img.data.get(),
                0))
            {
                throw std::runtime_error("Could not write \"" + match[2].str() + '"');
            }
            return true;
        }
        if (Mlib::re::regex_match(line, match, add_companion_renderable_reg)) {
            scene_node_resources.add_companion(
                match[1].str(),
                match[2].str(),
                { .regex = Mlib::compile_regex(match[3].str()) });
            return true;
        }
        if (Mlib::re::regex_match(line, match, repeat_reg)) {
            while (true) {
                std::cerr << "-";
                macro_line_executor(match[1].str(), local_substitutions, rsc);
            }
            return true;
        }

        auto cit = renderable_scenes.find(context);
        if (cit == renderable_scenes.end()) {
            throw std::runtime_error("Could not find renderable scene with name \"" + context + '"');
        }
        auto primary_rendering_context = cit->second->primary_rendering_context_;
        auto secondary_rendering_context = cit->second->secondary_rendering_context_;
        RenderingContextGuard rrg0{ primary_rendering_context };
        RenderingContextGuard rrg1{ secondary_rendering_context };

        auto primary_audio_context = cit->second->primary_audio_resource_context_;
        auto secondary_audio_context = cit->second->secondary_audio_resource_context_;
        AudioResourceContextGuard arg0{ primary_audio_context };
        AudioResourceContextGuard arg1{ secondary_audio_context };

        auto& scene_node_resources = cit->second->scene_node_resources_;
        auto& players = cit->second->players_;
        auto& scene = cit->second->scene_;
        auto& physics_engine = cit->second->physics_engine_;
        auto& button_press = cit->second->button_press_;
        auto& key_bindings = *cit->second->key_bindings_;
        auto& selected_cameras = cit->second->selected_cameras_;
        auto& scene_config = cit->second->scene_config_;
        auto& render_logics = cit->second->render_logics_;
        auto& physics_set_fps = cit->second->physics_set_fps_;
        auto& scene_logic = cit->second->standard_camera_logic_;
        auto& read_pixels_logic = cit->second->read_pixels_logic_;
        auto& dirtmap_logic = *cit->second->dirtmap_logic_;
        auto& post_processing_logic = *cit->second->post_processing_logic_;
        auto& skybox_logic = cit->second->skybox_logic_;
        auto& game_logic = cit->second->game_logic_;
        auto& base_log = cit->second->fifo_log_;
        auto& delete_node_mutex = cit->second->delete_node_mutex_;

        Linker linker{physics_engine.advance_times_};
        if (Mlib::re::regex_match(line, match, root_node_instance_reg)) {
            // 1: type
            // 2: name
            // 3, 4, 5: position
            // 6, 7, 8: rotation
            // 9: scale
            auto node = std::make_unique<SceneNode>(&scene);
            node->set_position(parse_position(
                scene_node_resources.get_geographic_mapping("world.inverse"),
                match[3].str(),
                match[4].str(),
                match[5].str()));
            node->set_rotation(FixedArray<float, 3>{
                safe_stof(match[6].str()) / 180.f * float(M_PI),
                safe_stof(match[7].str()) / 180.f * float(M_PI),
                safe_stof(match[8].str()) / 180.f * float(M_PI)});
            if (match[9].matched) {
                node->set_scale(safe_stof(match[9].str()));
            }
            std::string type = match[1].str();
            if (type == "aggregate") {
                scene.add_root_aggregate_node(match[2].str(), std::move(node));
            } else if (type == "instances") {
                scene.add_root_instances_node(match[2].str(), std::move(node));
            } else if (type == "static") {
                scene.add_static_root_node(match[2].str(), std::move(node));
            } else if (type == "dynamic") {
                scene.add_root_node(match[2].str(), std::move(node));
            } else {
                throw std::runtime_error("Unknown root node type: " + type);
            }
        } else if (Mlib::re::regex_match(line, match, delete_root_node_reg)) {
            std::lock_guard node_lock{ delete_node_mutex };
            scene.delete_root_node(match[1].str());
        } else if (Mlib::re::regex_match(line, match, delete_root_nodes_reg)) {
            std::lock_guard node_lock{ delete_node_mutex };
            scene.delete_root_nodes(Mlib::compile_regex(match[1].str()));
        } else if (Mlib::re::regex_match(line, match, delete_scheduled_advance_times_reg)) {
            physics_engine.advance_times_.delete_scheduled_advance_times();
        } else if (Mlib::re::regex_match(line, match, renderable_instance_reg)) {
            auto node = scene.get_node(match[2].str());
            scene_node_resources.instantiate_renderable(
                match[3].str(),
                match[1].str(),
                *node,
                { .regex = Mlib::compile_regex(match[4].str()) });
        } else if (Mlib::re::regex_match(line, match, register_geographic_mapping_reg)) {
            auto node = scene.get_node(match[2].str());
            scene_node_resources.register_geographic_mapping(
                match[3].str(),
                match[1].str(),
                *node);
        } else if (Mlib::re::regex_match(line, match, gun_reg)) {
            auto parent_rb_node = scene.get_node(match[2].str());
            auto rb = dynamic_cast<RigidBodyVehicle*>(parent_rb_node->get_absolute_movable());
            if (rb == nullptr) {
                throw std::runtime_error("Absolute movable is not a rigid body");
            }
            auto punch_angle_node = scene.get_node(match[3].str());
            std::shared_ptr<Gun> gun = std::make_shared<Gun>(
                scene,                                           // scene
                scene_node_resources,                            // scene_node_resources
                physics_engine.rigid_bodies_,                    // rigid_bodies
                physics_engine.advance_times_,                   // advance_times
                safe_stof(match[4].str()),                       // cool_down
                rb->rbi_,                                        // parent_rigid_body_node
                *punch_angle_node,                               // punch_angle_node
                match[5].str(),                                  // bullet-renderable-resource-name
                match[6].str(),                                  // bullet-hitbox-resource-name
                safe_stof(match[7].str()),                       // bullet-mass
                safe_stof(match[8].str()),                       // bullet_velocity
                safe_stof(match[9].str()),                       // bullet-lifetime
                safe_stof(match[10].str()),                      // bullet-damage
                FixedArray<float, 3>{                            // bullet-size
                    safe_stof(match[11].str()),                  // bullet-size-x
                    safe_stof(match[12].str()),                  // bullet-size-y
                    safe_stof(match[13].str())},                 // bullet-size-z
                safe_stof(match[14].str()) * float(M_PI / 180),  // punch_angle
                delete_node_mutex);                              // delete_node_mutex
                
            linker.link_absolute_observer(*scene.get_node(match[1].str()), gun);
        } else if (Mlib::re::regex_match(line, match, damageable_reg)) {
            auto rb = dynamic_cast<RigidBodyVehicle*>(scene.get_node(match[1].str())->get_absolute_movable());
            if (rb == nullptr) {
                throw std::runtime_error("Absolute movable is not a rigid body");
            }
            auto d = std::make_shared<DeletingDamageable>(
                scene,
                physics_engine.advance_times_,
                match[1].str(),
                safe_stof(match[2].str()),
                delete_node_mutex);
            physics_engine.advance_times_.add_advance_time(d);
            if (rb->damageable_ != nullptr) {
                throw std::runtime_error("Rigid body already has a damageable");
            }
            rb->damageable_ = d.get();
        } else if (Mlib::re::regex_match(line, match, crash_reg)) {
            auto rb = dynamic_cast<RigidBodyVehicle*>(scene.get_node(match[1].str())->get_absolute_movable());
            if (rb == nullptr) {
                throw std::runtime_error("Absolute movable is not a rigid body");
            }
            auto d = std::make_shared<Crash>(
                *rb,
                safe_stof(match[2].str()));  // damage
            rb->collision_observers_.push_back(d);
        } else if (Mlib::re::regex_match(line, match, relative_transformer_reg)) {
            FixedArray<float, 3> v{
                match[2].str().empty() ? 0.f : safe_stof(match[2].str()),
                match[3].str().empty() ? 0.f : safe_stof(match[3].str()),
                match[4].str().empty() ? 0.f : safe_stof(match[4].str())};
            FixedArray<float, 3> w{
                match[5].str().empty() ? 0.f : safe_stof(match[5].str()) * float(M_PI / 180),
                match[6].str().empty() ? 0.f : safe_stof(match[6].str()) * float(M_PI / 180),
                match[7].str().empty() ? 0.f : safe_stof(match[7].str()) * float(M_PI / 180)};
            std::shared_ptr<RelativeTransformer> rt = std::make_shared<RelativeTransformer>(
                physics_engine.advance_times_, v, w);
            linker.link_relative_movable(*scene.get_node(match[1].str()), rt);
        } else if (Mlib::re::regex_match(line, match, create_engine_reg)) {
            auto rb = dynamic_cast<RigidBodyVehicle*>(scene.get_node(match[1].str())->get_absolute_movable());
            if (rb == nullptr) {
                throw std::runtime_error("Absolute movable is not a rigid body");
            }
            EnginePower engine_power{
                Interp<float>{
                    string_to_vector(match[3].str(), safe_stof),
                    string_to_vector(match[4].str(), safe_stof),
                    OutOfRangeBehavior::CLAMP},
                string_to_vector(match[5].str(), safe_stof)};
            auto ep = rb->engines_.insert({
                match[2].str(),
                RigidBodyEngine{
                    engine_power,
                    match[6].str().empty() ? false : safe_stob(match[6].str()),  // hand_brake_pulled
                    match[7].matched ? std::make_shared<EngineAudio>(match[7].str()) : nullptr}});
            if (!ep.second) {
                throw std::runtime_error("Engine with name \"" + match[2].str() + "\" already exists");
            }
        } else if (Mlib::re::regex_match(line, match, clear_renderable_instance_reg)) {
            scene.get_node(match[1].str())->clear_renderable_instance(match[2].str());
        } else if (Mlib::re::regex_match(line, match, clear_absolute_observer_reg)) {
            scene.get_node(match[1].str())->clear_absolute_observer();
        } else if (Mlib::re::regex_match(line, match, player_set_aiming_gun_reg)) {
            auto ypln_node = scene.get_node(match[2].str());
            auto ypln = dynamic_cast<YawPitchLookAtNodes*>(ypln_node->get_relative_movable());
            if (ypln == nullptr) {
                throw std::runtime_error("Relative movable is not a ypln");
            }
            SceneNode* gun_node = nullptr;
            if (!match[3].str().empty()) {
                gun_node = scene.get_node(match[3].str());
            }
            players.get_player(match[1].str()).set_ypln(*ypln, gun_node);
        } else if (Mlib::re::regex_match(line, match, player_set_surface_power_reg)) {
            players.get_player(match[1].str()).set_surface_power(
                safe_stof(match[2].str()),
                safe_stof(match[3].str()));
        } else if (Mlib::re::regex_match(line, match, player_set_waypoint_reg)) {
            players.get_player(match[1].str()).set_waypoint({
                safe_stof(match[2].str()),
                safe_stof(match[3].str()),
                safe_stof(match[4].str())});
        } else if (Mlib::re::regex_match(line, match, team_set_waypoint_reg)) {
            players.set_team_waypoint(
                match[1].str(), {
                    safe_stof(match[2].str()),
                    safe_stof(match[3].str()),
                    safe_stof(match[4].str())});
        } else if (Mlib::re::regex_match(line, match, camera_key_binding_reg)) {
            key_bindings.add_camera_key_binding(CameraKeyBinding{
                .base = {
                    .key = match[1].str(),
                    .gamepad_button = match[2].str(),
                    .joystick_axis = match[3].str(),
                    .joystick_axis_sign = match[4].matched
                        ? safe_stof(match[4].str())
                        : 0.f}});
        } else if (Mlib::re::regex_match(line, match, abs_idle_binding_reg)) {
            key_bindings.add_absolute_movable_idle_binding(AbsoluteMovableIdleBinding{
                .node = scene.get_node(match[1].str()),
                .tires_z = {
                    match[2].str().empty() ? 0.f : safe_stof(match[2].str()),
                    match[3].str().empty() ? 0.f : safe_stof(match[3].str()),
                    match[4].str().empty() ? 1.f : safe_stof(match[4].str())}});
        } else if (Mlib::re::regex_match(line, match, weapon_inventory_key_binding_reg)) {
            try {
                scene.get_node(match[1].str())->get_node_modifier();
            } catch (const std::runtime_error& e) {
                throw std::runtime_error("Node \"" + match[1].str() + "\": " + e.what());
            }
            key_bindings.add_weapon_inventory_key_binding(WeaponInventoryKeyBinding{
                .base_key = {
                    .key = match[2].str(),
                    .gamepad_button = match[3].str(),
                    .joystick_axis = match[4].str(),
                    .joystick_axis_sign = safe_stof(match[5].str())},
                .base_scroll_wheel_axis = {
                    .axis = match[6].matched ? safe_stou(match[6].str()) : SIZE_MAX,
                    .sign_and_scale = match[7].matched ? safe_stof(match[7].str()) : NAN,
                },
                .scroll_wheel_movement = std::make_shared<CursorMovement>(scroll_wheel_states),
                .node = scene.get_node(match[1].str()),
                .direction = safe_stoi(match[8].str())});
        } else if (Mlib::re::regex_match(line, match, console_log_reg)) {
            auto node = scene.get_node(match[1].str());
            auto mv = node->get_absolute_movable();
            auto lo = dynamic_cast<StatusWriter*>(mv);
            if (lo == nullptr) {
                throw std::runtime_error("Could not find loggable");
            }
            StatusComponents log_components = (StatusComponents)safe_stou(match[2].str());
            auto logger = std::make_shared<MovableLogger>(*node, physics_engine.advance_times_, lo, log_components);
            physics_engine.advance_times_.add_advance_time(logger);
        } else if (Mlib::re::regex_match(line, match, visual_global_log_reg)) {
            auto logger = std::make_shared<VisualGlobalLog>(
                base_log,
                fpathp(match[1].str()),
                FixedArray<float, 2>{
                    safe_stof(match[2].str()),
                    safe_stof(match[3].str())},
                safe_stof(match[4].str()),
                safe_stof(match[5].str()),
                safe_stoz(match[6].str()),
                log_entry_severity_from_string(match[7].str()));
            render_logics.append(nullptr, logger);
        } else if (Mlib::re::regex_match(line, match, visual_node_status_3rd_reg)) {
            auto node = scene.get_node(match[1].str());
            auto mv = node->get_absolute_movable();
            auto lo = dynamic_cast<StatusWriter*>(mv);
            if (lo == nullptr) {
                throw std::runtime_error("Could not find loggable");
            }
            StatusComponents log_components = (StatusComponents)safe_stoi(match[2].str());
            auto logger = std::make_shared<VisualMovable3rdLogger>(
                scene_logic,
                *node,
                physics_engine.advance_times_,
                lo,
                log_components,
                fpathp(match[3].str()),
                FixedArray<float, 2>{
                    safe_stof(match[4].str()),
                    safe_stof(match[5].str())},
                safe_stof(match[6].str()),
                safe_stof(match[7].str()));
            render_logics.append(node, logger);
            physics_engine.advance_times_.add_advance_time(logger);
        } else if (Mlib::re::regex_match(line, match, countdown_reg)) {
            auto countdown_logic = std::make_shared<CountDownLogic>(
                fpathp(match[1].str()),            // ttf_filename
                FixedArray<float, 2>{              // position
                    safe_stof(match[2].str()),
                    safe_stof(match[3].str())},
                safe_stof(match[4].str()),         // font_height_pixels
                safe_stof(match[5].str()),         // line_distance_pixels
                ui_focus.focuses,
                safe_stof(match[6].str()));        // nseconds
            RenderingContextGuard rcg{ RenderingContext {.rendering_resources = secondary_rendering_context.rendering_resources, .z_order = 1} };
            render_logics.append(nullptr, countdown_logic);
        } else if (Mlib::re::regex_match(line, match, focused_text_reg)) {
            auto loading_logic = std::make_shared<FocusedTextLogic>(
                fpathp(match[1].str()),            // ttf_filename
                FixedArray<float, 2>{              // position
                    safe_stof(match[2].str()),
                    safe_stof(match[3].str())},
                safe_stof(match[4].str()),         // font_height_pixels
                safe_stof(match[5].str()),         // line_distance_pixels
                focus_from_string(match[6].str()), // focus mask
                match[7].str());                   // text
            RenderingContextGuard rcg{ RenderingContext {.rendering_resources = secondary_rendering_context.rendering_resources, .z_order = 1} };
            render_logics.append(nullptr, loading_logic);
        } else if (Mlib::re::regex_match(line, match, players_stats_reg)) {
            auto players_stats_logic = std::make_shared<PlayersStatsLogic>(
                players,
                fpathp(match[1].str()),                               // ttf_filename
                FixedArray<float, 2>{                                 // position
                    safe_stof(match[2].str()),
                    safe_stof(match[3].str())},
                safe_stof(match[4].str()),                            // font_height_pixels
                safe_stof(match[5].str()),                            // line_distance_pixels
                (ScoreBoardConfiguration)safe_stoi(match[6].str()));  // score board configuration
            render_logics.append(nullptr, players_stats_logic);
        } else if (Mlib::re::regex_match(line, match, pause_on_lose_focus_reg)) {
            auto wit = renderable_scenes.find("primary_scene");
            if (wit == renderable_scenes.end()) {
                throw std::runtime_error("Could not find renderable scene with name \"primary_scene\"");
            }
            auto polf = std::make_shared<PauseOnLoseFocusLogic>(
                physics_set_fps,
                ui_focus,
                FocusFilter{
                    .focus_mask = focus_from_string(match[1].str()),
                    .submenu_id = match[2].str() });
            wit->second->render_logics_.append(nullptr, polf);
        } else if (Mlib::re::regex_match(line, match, scene_selector_reg)) {
            std::list<SceneEntry> scene_entries;
            for (const auto& e : find_all_name_values(match[8].str(), "[\\w+-. \\(\\)/:]+", "[\\w+-. \\(\\)/:]+")) {
                scene_entries.push_back(SceneEntry{
                    .name = e.first,
                    .filename = fpathp(e.second)});
            }
            std::string id = match[1].str();
            std::string title = match[2].str();
            ui_focus.insert_submenu(id, title, 0);
            auto scene_selector_logic = std::make_shared<SceneSelectorLogic>(
                "",
                std::vector<SceneEntry>{scene_entries.begin(), scene_entries.end()},
                fpathp(match[3].str()),            // ttf_filename
                FixedArray<float, 2>{             // position
                    safe_stof(match[4].str()),
                    safe_stof(match[5].str())},
                safe_stof(match[6].str()),        // font_height_pixels
                safe_stof(match[7].str()),        // line_distance_pixels
                FocusFilter{
                    .focus_mask = Focus::MENU,
                    .submenu_id = id },
                next_scene_filename,
                button_press,
                ui_focus.selection_ids.at(id));
            RenderingContextGuard rcg{ RenderingContext {.rendering_resources = secondary_rendering_context.rendering_resources, .z_order = 1} };
            render_logics.append(nullptr, scene_selector_logic);
        } else if (Mlib::re::regex_match(line, match, scene_to_texture_reg)) {
            auto wit = renderable_scenes.find("primary_scene");
            if (wit == renderable_scenes.end()) {
                throw std::runtime_error("Could not find renderable scene with name \"primary_scene\"");
            }
            auto scene_window_logic = std::make_shared<RenderToTextureLogic>(
                render_logics,                    // child_logic
                resource_update_cycle_from_string(match[2].str()),
                false,                            // with_depth_texture
                match[1].str(),                   // color_texture_name
                "",                               // depth_texture_name
                safe_stoi(match[3].str()),        // texture_width
                safe_stoi(match[4].str()),        // texture_height
                FocusFilter{
                    .focus_mask = focus_from_string(match[5].str()),
                    .submenu_id = match[6].str() });
            wit->second->render_logics_.prepend(nullptr, scene_window_logic);
        } else if (Mlib::re::regex_match(line, match, fill_pixel_region_with_texture_reg)) {
            std::string source_scene = match[1].str();
            auto wit = renderable_scenes.find(source_scene);
            if (wit == renderable_scenes.end()) {
                throw std::runtime_error("Could not find renderable scene with name \"" + source_scene + '"');
            }
            std::shared_ptr<FillPixelRegionWithTextureLogic> scene_window_logic;
            {
                RenderingContextGuard rcg{wit->second->secondary_rendering_context_};
                scene_window_logic = std::make_shared<FillPixelRegionWithTextureLogic>(
                    match[2].str(),                   // texture name
                    resource_update_cycle_from_string(match[3].str()),
                    FixedArray<float, 2>{             // position
                        safe_stof(match[4].str()),
                        safe_stof(match[5].str())},
                    FixedArray<float, 2>{             // size
                        safe_stof(match[6].str()),
                        safe_stof(match[7].str())},
                    FocusFilter{
                        .focus_mask = focus_from_string(match[8].str()),
                        .submenu_id = match[9].str()});
            }
            render_logics.append(nullptr, scene_window_logic);
        } else if (Mlib::re::regex_match(line, match, scene_to_pixel_region_reg)) {
            std::string target_scene = match[1].str();
            auto wit = renderable_scenes.find(target_scene);
            if (wit == renderable_scenes.end()) {
                throw std::runtime_error("Could not find renderable scene with name \"" + target_scene + '"');
            }
            std::shared_ptr<RenderToPixelRegionLogic> render_scene_to_pixel_region_logic_;
            render_scene_to_pixel_region_logic_ = std::make_shared<RenderToPixelRegionLogic>(
                render_logics,
                FixedArray<int, 2>{             // position
                    safe_stoi(match[2].str()),
                    safe_stoi(match[3].str())},
                FixedArray<int, 2>{             // size
                    safe_stoi(match[4].str()),
                    safe_stoi(match[5].str())},
                FocusFilter{
                    .focus_mask = focus_from_string(match[6].str()),
                    .submenu_id = match[7].str()});
            RenderingContextGuard rcg{ RenderingContext {.rendering_resources = secondary_rendering_context.rendering_resources, .z_order = 1} };
            wit->second->render_logics_.append(nullptr, render_scene_to_pixel_region_logic_);
        } else if (Mlib::re::regex_match(line, match, controls_reg)) {
            std::string id = match[1].str();
            std::string title = match[2].str();
            std::shared_ptr<ControlsLogic> controls_logic;
            ui_focus.insert_submenu(id, title, 0);
            controls_logic = std::make_shared<ControlsLogic>(
                fpathp(match[3].str()),            // gamepad_texture
                FixedArray<float, 2>{             // position
                    safe_stof(match[4].str()),
                    safe_stof(match[5].str())},
                FixedArray<float, 2>{             // size
                    safe_stof(match[6].str()),
                    safe_stof(match[7].str())},
                FocusFilter{
                    .focus_mask = Focus::MENU,
                    .submenu_id = id });
            RenderingContextGuard rcg{ RenderingContext {.rendering_resources = secondary_rendering_context.rendering_resources, .z_order = 1} };
            render_logics.append(nullptr, controls_logic);
        } else if (Mlib::re::regex_match(line, match, clear_parameters_reg)) {
            external_substitutions.clear();
        } else if (Mlib::re::regex_match(line, match, parameter_setter_reg)) {
            std::string id = match[1].str();
            std::string title = match[2].str();
            std::string ttf_filename = fpathp(match[3].str());
            FixedArray<float, 2> position{
                safe_stof(match[4].str()),
                safe_stof(match[5].str()) };
            float font_height_pixels = safe_stof(match[6].str());
            float line_distance_pixels = safe_stof(match[7].str());
            size_t deflt = safe_stoz(match[8].str());
            std::string on_init = match[9].str();
            std::string on_change = match[10].str();
            std::string parameters = match[11].str();
            std::list<ReplacementParameter> rps;
            for (const auto& e : find_all_name_values(parameters, "[\\w+-. %]+", substitute_pattern)) {
                rps.push_back(ReplacementParameter{
                    .name = e.first,
                    .substitutions = SubstitutionMap{ replacements_to_map(e.second) } });
            }
            ui_focus.insert_submenu(id, title, deflt);
            auto parameter_setter_logic = std::make_shared<ParameterSetterLogic>(
                "",
                std::vector<ReplacementParameter>{rps.begin(), rps.end()},
                ttf_filename,
                position,
                font_height_pixels,
                line_distance_pixels,        // line_distance_pixels
                FocusFilter{
                    .focus_mask = Focus::MENU,
                    .submenu_id = id },
                external_substitutions,
                button_press,
                ui_focus.selection_ids.at(id),
                [macro_line_executor, on_change, &rsc]() {
                    if (!on_change.empty()) {
                        macro_line_executor(on_change, nullptr, rsc);
                    }
                });
            if (!on_init.empty()) {
                macro_line_executor(on_init, local_substitutions, rsc);
            }
            RenderingContextGuard rcg{ RenderingContext {.rendering_resources = secondary_rendering_context.rendering_resources, .z_order = 1} };
            render_logics.append(nullptr, parameter_setter_logic);
        } else if (Mlib::re::regex_match(line, match, ui_background_reg)) {
            auto bg = std::make_shared<MainMenuBackgroundLogic>(
                fpathp(match[1].str()),
                resource_update_cycle_from_string(match[2].str()),
                FocusFilter{ .focus_mask = focus_from_string(match[3].str()) });
            RenderingContextGuard rcg{ RenderingContext {.rendering_resources = secondary_rendering_context.rendering_resources, .z_order = 1} };
            render_logics.append(nullptr, bg);
        } else if (Mlib::re::regex_match(line, match, set_renderable_style_reg)) {
            auto node = scene.get_node(match[2].str());
            node->set_style(std::unique_ptr<Style>(new Style{
                .selector = Mlib::compile_regex(match[1].str()),
                .ambience = {
                    safe_stof(match[3].str()),
                    safe_stof(match[4].str()),
                    safe_stof(match[5].str())},
                .diffusivity = {
                    safe_stof(match[6].str()),
                    safe_stof(match[7].str()),
                    safe_stof(match[8].str())},
                .specularity = {
                    safe_stof(match[9].str()),
                    safe_stof(match[10].str()),
                    safe_stof(match[11].str())},
                .skelletal_animation_name = match[12].matched ? match[12].str() : "",
                .skelletal_animation_frame = {
                    .begin = match[13].matched ? safe_stof(match[13].str()) : NAN,
                    .end = match[14].matched ? safe_stof(match[14].str()) : NAN,
                    .time = match[15].matched ? safe_stof(match[15].str()) : NAN}}));
        } else if (Mlib::re::regex_match(line, match, set_avatar_style_updater_reg)) {
            auto avatar_node = scene.get_node(match[1].str());
            auto gun_node = scene.get_node(match[2].str());
            std::string resource_wo_gun = match[3].str();
            std::string resource_w_gun = match[4].str();
            auto rb = dynamic_cast<RigidBodyVehicle*>(avatar_node->get_absolute_movable());
            if (rb == nullptr) {
                throw std::runtime_error("Styled node movable is not a rigid body");
            }
            if (rb->style_updater_ != nullptr) {
                throw std::runtime_error("Rigid body already has a style updater");
            }
            auto updater = std::make_unique<AvatarAnimationUpdater>(*rb, *gun_node, resource_wo_gun, resource_w_gun);
            StyleUpdater* ptr = updater.get();
            avatar_node->set_style_updater(std::move(updater));
            rb->style_updater_ = ptr;
        } else if (Mlib::re::regex_match(line, match, hud_image_reg)) {
            auto camera_node = scene.get_node(match[2].str());
            auto hud_image = std::make_shared<HudImageLogic>(
                *camera_node,
                physics_engine.advance_times_,
                fpathp(match[3].str()),
                resource_update_cycle_from_string(match[4].str()),
                FixedArray<float, 2>{
                    safe_stof(match[5].str()),
                    safe_stof(match[6].str())},
                FixedArray<float, 2>{
                    safe_stof(match[7].str()),
                    safe_stof(match[8].str())});
            render_logics.append(camera_node, hud_image);
            camera_node->add_renderable(match[1].str(), hud_image);
            physics_engine.advance_times_.add_advance_time(hud_image);
        } else if (Mlib::re::regex_match(line, match, perspective_camera_reg)) {
            auto node = scene.get_node(match[1].str());
            node->set_camera(std::make_unique<GenericCamera>(scene_config.camera_config, GenericCamera::Mode::PERSPECTIVE));
            node->get_camera()->set_y_fov(safe_stof(match[2].str()) / 180.f * float(M_PI));
            node->get_camera()->set_near_plane(safe_stof(match[3].str()));
            node->get_camera()->set_far_plane(safe_stof(match[4].str()));
            node->get_camera()->set_requires_postprocessing(safe_stoi(match[5].str()));
        } else if (Mlib::re::regex_match(line, match, ortho_camera_reg)) {
            auto node = scene.get_node(match[1].str());
            node->set_camera(std::make_unique<GenericCamera>(scene_config.camera_config, GenericCamera::Mode::ORTHO));
            node->get_camera()->set_near_plane(safe_stof(match[2].str()));
            node->get_camera()->set_far_plane(safe_stof(match[3].str()));
            node->get_camera()->set_left_plane(safe_stof(match[4].str()));
            node->get_camera()->set_right_plane(safe_stof(match[5].str()));
            node->get_camera()->set_bottom_plane(safe_stof(match[6].str()));
            node->get_camera()->set_top_plane(safe_stof(match[7].str()));
            node->get_camera()->set_requires_postprocessing(safe_stoi(match[8].str()));
        } else if (Mlib::re::regex_match(line, match, light_reg)) {
            std::lock_guard lock_guard{ delete_node_mutex };
            std::string node_name = match[1].str();
            SceneNode* node = scene.get_node(node_name);
            render_logics.prepend(node, std::make_shared<LightmapLogic>(
                read_pixels_logic,
                resource_update_cycle_from_string(match[3].str()),
                node_name,
                match[2].str(),               // black_node_name
                safe_stob(match[4].str())));  // with_depth_texture
            node->add_light(std::make_unique<Light>(Light{
                .ambience = {
                    safe_stof(match[5].str()),
                    safe_stof(match[6].str()),
                    safe_stof(match[7].str())},
                .diffusivity = {
                    safe_stof(match[8].str()),
                    safe_stof(match[9].str()),
                    safe_stof(match[10].str())},
                .specularity = {
                    safe_stof(match[11].str()),
                    safe_stof(match[12].str()),
                    safe_stof(match[13].str())},
                .node_name = node_name,
                .only_black = !match[2].str().empty(),
                .shadow = safe_stob(match[14].str())}));
        } else if (Mlib::re::regex_match(line, match, look_at_node_reg)) {
            auto follower_node = scene.get_node(match[1].str());
            auto followed_node = scene.get_node(match[2].str());
            auto follower = std::make_shared<LookAtMovable>(
                physics_engine.advance_times_,
                scene,
                match[1].str(),
                followed_node,
                followed_node->get_absolute_movable());
            linker.link_absolute_movable(*follower_node, follower);
        } else if (Mlib::re::regex_match(line, match, keep_offset_reg)) {
            auto follower_node = scene.get_node(match[1].str());
            auto followed_node = scene.get_node(match[2].str());
            auto follower = std::make_shared<KeepOffsetMovable>(
                physics_engine.advance_times_,
                scene,
                match[1].str(),
                followed_node,
                followed_node->get_absolute_movable(),
                FixedArray<float, 3>{
                    safe_stof(match[3].str()),
                    safe_stof(match[4].str()),
                    safe_stof(match[5].str())});
            linker.link_absolute_movable(*follower_node, follower);
        } else if (Mlib::re::regex_match(line, match, follow_node_reg)) {
            auto follower_node = scene.get_node(match[1].str());
            auto followed_node = scene.get_node(match[2].str());
            auto distance = safe_stof(match[3].str());
            auto follower = std::make_shared<FollowMovable>(
                physics_engine.advance_times_,
                followed_node,
                followed_node->get_absolute_movable(),
                distance,                          // attachment_distance
                FixedArray<float, 3>{              // node_displacement
                    safe_stof(match[4].str()),
                    safe_stof(match[5].str()),
                    safe_stof(match[6].str())},
                FixedArray<float, 3>{              // look_at_displacement
                    safe_stof(match[7].str()),
                    safe_stof(match[8].str()),
                    safe_stof(match[9].str())},
                safe_stof(match[10].str()),        // snappiness
                safe_stof(match[11].str()),        // y_adaptivity
                safe_stof(match[12].str()),        // y_snappiness
                scene_config.physics_engine_config.dt);
            linker.link_absolute_movable(*follower_node, follower);
            follower->initialize(*follower_node);
        } else if (Mlib::re::regex_match(line, match, record_track_reg)) {
            auto recorder_node = scene.get_node(match[1].str());
            auto rb = dynamic_cast<RigidBodyVehicle*>(recorder_node->get_absolute_movable());
            if (rb == nullptr) {
                throw std::runtime_error("Absolute movable is not a rigid body");
            }
            physics_engine.advance_times_.add_advance_time(std::make_shared<RigidBodyRecorder>(
                fpathp(match[2].str()),
                physics_engine.advance_times_,
                recorder_node,
                &rb->rbi_,
                ui_focus.focuses));
        } else if (Mlib::re::regex_match(line, match, record_track_gpx_reg)) {
            auto recorder_node = scene.get_node(match[1].str());
            auto rb = dynamic_cast<RigidBodyVehicle*>(recorder_node->get_absolute_movable());
            if (rb == nullptr) {
                throw std::runtime_error("Absolute movable is not a rigid body");
            }
            physics_engine.advance_times_.add_advance_time(std::make_shared<RigidBodyRecorderGpx>(
                fpathp(match[2].str()),
                physics_engine.advance_times_,
                recorder_node,
                &rb->rbi_,
                scene_node_resources.get_geographic_mapping("world"),
                ui_focus.focuses));
        } else if (Mlib::re::regex_match(line, match, playback_track_reg)) {
            auto playback_node = scene.get_node(match[1].str());
            auto playback = std::make_shared<RigidBodyPlayback>(
                fpathp(match[3].str()),
                physics_engine.advance_times_,
                ui_focus.focuses,
                safe_stof(match[2].str()));
            linker.link_absolute_movable(*playback_node, playback);
        } else if (Mlib::re::regex_match(line, match, define_winner_conditionals_reg)) {
            if (local_substitutions == nullptr) {
                throw std::runtime_error("Cannot define winner conditionals without local substitutions");
            }
            for (size_t rank = safe_stoz(match[1].str()); rank < safe_stoz(match[2].str()); ++rank) {
                LapTimeEventAndIdAndMfilename lapTimeEvent = players.get_winner_track_filename(rank);
                local_substitutions->merge(SubstitutionMap({
                    {
                        "IF_WINNER_RANK" + std::to_string(rank) + "_EXISTS",
                        lapTimeEvent.m_filename.empty() ? "# " : ""
                    },
                    {
                        "VEHICLE_WINNER" + std::to_string(rank),
                        lapTimeEvent.event.vehicle
                    }}));
            }
        } else if (Mlib::re::regex_match(line, match, playback_winner_track_reg)) {
            size_t rank = safe_stoz(match[3].str());
            std::string filename = players.get_winner_track_filename(rank).m_filename;
            if (filename.empty()) {
                throw std::runtime_error("Winner with rank " + std::to_string(rank) + " does not exist");
            }
            auto playback_node = scene.get_node(match[1].str());
            auto playback = std::make_shared<RigidBodyPlayback>(
                filename,
                physics_engine.advance_times_,
                ui_focus.focuses,
                safe_stof(match[2].str()));
            linker.link_absolute_movable(*playback_node, playback);
        } else if (Mlib::re::regex_match(line, match, check_points_reg)) {
            auto moving_node = scene.get_node(match[1].str());
            std::string on_finish = match[10].str();
            physics_engine.advance_times_.add_advance_time(std::make_shared<CheckPoints>(
                fpathp(match[9].str()),                  // filename
                physics_engine.advance_times_,
                moving_node,
                moving_node->get_absolute_movable(),
                match[2].str(),                         // resource
                &players.get_player(match[3].str()),
                safe_stoi(match[4].str()),              // nbeacons
                safe_stoi(match[5].str()),              // nth
                safe_stoi(match[6].str()),              // nahead
                safe_stof(match[7].str()),              // radius
                scene_node_resources,
                scene,
                delete_node_mutex,
                ui_focus.focuses,
                safe_stob(match[8].str()),              // enable_height_changed_mode
                [on_finish, macro_line_executor, &physics_set_fps, &rsc](){
                    macro_line_executor(on_finish, nullptr, rsc);
                }));
        } else if (Mlib::re::regex_match(line, match, set_camera_cycle_reg)) {
            std::string cameras = match[2].str();
            auto& cycle = (match[1].str() == "near")
                ? selected_cameras.camera_cycle_near
                : selected_cameras.camera_cycle_far;
            cycle = string_to_vector(cameras);
        } else if (Mlib::re::regex_match(line, match, set_camera_reg)) {
            selected_cameras.set_camera_node_name(match[1].str());
        } else if (Mlib::re::regex_match(line, match, set_dirtmap_reg)) {
            dirtmap_logic.set_filename(fpathp(match[1].str()));
            secondary_rendering_context.rendering_resources->set_offset("dirtmap", safe_stof(match[2].str()));
            secondary_rendering_context.rendering_resources->set_discreteness("dirtmap", safe_stof(match[3].str()));
            secondary_rendering_context.rendering_resources->set_scale("dirtmap", safe_stof(match[4].str()));
            secondary_rendering_context.rendering_resources->set_texture_wrap("dirtmap", wrap_mode_from_string(match[5].str()));
        } else if (Mlib::re::regex_match(line, match, set_soft_light_reg)) {
            post_processing_logic.set_soft_light_filename(fpathp(match[1].str()));
        } else if (Mlib::re::regex_match(line, match, set_skybox_reg)) {
            skybox_logic.set_filenames({
                fpathp(match[2].str()),
                fpathp(match[3].str()),
                fpathp(match[4].str()),
                fpathp(match[5].str()),
                fpathp(match[6].str()),
                fpathp(match[7].str())},
                match[1].str());
        } else if (Mlib::re::regex_match(line, match, set_vip_reg)) {
            game_logic.bystanders.set_vip(&players.get_player(match[1].str()));
        } else if (Mlib::re::regex_match(line, match, respawn_all_players_reg)) {
            game_logic.spawn.respawn_all_players();
        } else if (Mlib::re::regex_match(line, match, set_way_points_reg)) {
            Player& player = players.get_player(match[1].str());
            SceneNode* node = scene.get_node(match[2].str());
            std::map<WayPointLocation, PointsAndAdjacency<float, 3>> way_points = scene_node_resources.way_points(match[3].str());
            player.set_waypoints(*node, way_points);
        } else if (Mlib::re::regex_match(line, match, set_pod_bot_way_points_reg)) {
            SceneNode* node = scene.get_node(match[1].str());
            std::map<WayPointLocation, PointsAndAdjacency<float, 3>> way_points = scene_node_resources.way_points(match[2].str());
            set_pod_bot_way_points(*node, way_points);
        } else if (Mlib::re::regex_match(line, match, set_pod_bot_game_mode_reg)) {
            set_pod_bot_game_mode(match[1].str());
        } else if (Mlib::re::regex_match(line, match, set_can_drive_reg)) {
            Player& player = players.get_player(match[1].str());
            player.set_can_drive(safe_stob(match[2].str()));
        } else if (Mlib::re::regex_match(line, match, set_can_aim_reg)) {
            Player& player = players.get_player(match[1].str());
            player.set_can_aim(safe_stob(match[2].str()));
        } else if (Mlib::re::regex_match(line, match, set_can_shoot_reg)) {
            Player& player = players.get_player(match[1].str());
            player.set_can_shoot(safe_stob(match[2].str()));
        } else if (Mlib::re::regex_match(line, match, burn_in_reg)) {
            physics_engine.burn_in(safe_stof(match[1].str()));
            scene.move(0.f); // dt
        } else if (Mlib::re::regex_match(line, match, add_node_not_allowed_to_be_unregistered_reg)) {
            scene.add_node_not_allowed_to_be_unregistered(match[1].str());
        } else if (Mlib::re::regex_match(line, match, remove_node_not_allowed_to_be_unregistered_reg)) {
            scene.remove_node_not_allowed_to_be_unregistered(match[1].str());
        } else if (Mlib::re::regex_match(line, match, clear_nodes_not_allowed_to_be_unregistered_reg)) {
            scene.clear_nodes_not_allowed_to_be_unregistered();
        } else if (Mlib::re::regex_match(line, match, execute_in_physics_thread_reg)) {
            std::string command = match[1].str();
            physics_set_fps.execute([macro_line_executor, command, &rsc](){macro_line_executor(command, nullptr, rsc);});
        } else {
            return false;
        }
        return true;
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
