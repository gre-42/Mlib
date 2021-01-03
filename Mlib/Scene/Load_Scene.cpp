#include "Load_Scene.hpp"
#include <Mlib/Geometry/Mesh/Load_Mesh_Config.hpp>
#include <Mlib/Math/Pi.hpp>
#include <Mlib/Math/Rodrigues.hpp>
#include <Mlib/Physics/Advance_Times/Check_Points.hpp>
#include <Mlib/Physics/Advance_Times/Crash.hpp>
#include <Mlib/Physics/Advance_Times/Deleting_Damageable.hpp>
#include <Mlib/Physics/Advance_Times/Game_Logic.hpp>
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
#include <Mlib/Physics/Advance_Times/Player.hpp>
#include <Mlib/Physics/Advance_Times/Rigid_Body_Recorder.hpp>
#include <Mlib/Physics/Advance_Times/Trigger_Gun_Ai.hpp>
#include <Mlib/Physics/Containers/Players.hpp>
#include <Mlib/Physics/Misc/Rigid_Body.hpp>
#include <Mlib/Physics/Misc/Rigid_Body_Engine.hpp>
#include <Mlib/Physics/Misc/Rigid_Primitives.hpp>
#include <Mlib/Physics/Physics_Engine.hpp>
#include <Mlib/Regex.hpp>
#include <Mlib/Render/Cameras/Generic_Camera.hpp>
#include <Mlib/Render/Render_Logics/Countdown_Logic.hpp>
#include <Mlib/Render/Render_Logics/Dirtmap_Logic.hpp>
#include <Mlib/Render/Render_Logics/Lightmap_Logic.hpp>
#include <Mlib/Render/Render_Logics/Loading_Text_Logic.hpp>
#include <Mlib/Render/Render_Logics/Main_Menu_Background_Logic.hpp>
#include <Mlib/Render/Render_Logics/Read_Pixels_Logic.hpp>
#include <Mlib/Render/Render_Logics/Render_Logics.hpp>
#include <Mlib/Render/Render_Logics/Skybox_Logic.hpp>
#include <Mlib/Render/Renderables/Renderable_Binary_X.hpp>
#include <Mlib/Render/Renderables/Renderable_Blending_X.hpp>
#include <Mlib/Render/Renderables/Renderable_Obj_File.hpp>
#include <Mlib/Render/Renderables/Renderable_Osm_Map.hpp>
#include <Mlib/Render/Renderables/Renderable_Square.hpp>
#include <Mlib/Render/Rendering_Resources.hpp>
#include <Mlib/Render/Selected_Cameras.hpp>
#include <Mlib/Render/Ui/Button_Press.hpp>
#include <Mlib/Scene/Render_Logics/Hud_Image_Logic.hpp>
#include <Mlib/Scene/Render_Logics/Key_Bindings.hpp>
#include <Mlib/Scene/Render_Logics/Parameter_Setter_Logic.hpp>
#include <Mlib/Scene/Render_Logics/Players_Stats_Logic.hpp>
#include <Mlib/Scene/Render_Logics/Scene_Selector_Logic.hpp>
#include <Mlib/Scene/Render_Logics/Visual_Global_Log.hpp>
#include <Mlib/Scene/Render_Logics/Visual_Movable_3rd_Logger.hpp>
#include <Mlib/Scene/Render_Logics/Visual_Movable_Logger.hpp>
#include <Mlib/Scene_Graph/Base_Log.hpp>
#include <Mlib/Scene_Graph/Driving_Direction.hpp>
#include <Mlib/Scene_Graph/Log_Entry_Severity.hpp>
#include <Mlib/Scene_Graph/Scene.hpp>
#include <Mlib/Scene_Graph/Scene_Node_Resources.hpp>
#include <Mlib/Scene_Graph/Style.hpp>
#include <Mlib/String.hpp>
#include <filesystem>
#include <fstream>
#include <regex>

namespace fs = std::filesystem;

using namespace Mlib;

class Linker {
public:
    explicit Linker(AdvanceTimes& advance_times)
    : advance_times_{advance_times}
    {}

    template <class TAbsoluteMovable>
    void link_absolute_movable(SceneNode& node, const std::shared_ptr<TAbsoluteMovable>& absolute_movable) const {
        // 1. Set movable, which updates the transformation-matrix
        node.set_absolute_movable(absolute_movable.get());
        // 2. Add to physics engine
        advance_times_.add_advance_time(absolute_movable);
    };
    template <class TRelativeMovable>
    void link_relative_movable(SceneNode& node, const std::shared_ptr<TRelativeMovable>& relative_movable) const {
        node.set_relative_movable(relative_movable.get());
        advance_times_.add_advance_time(relative_movable);
    };
    template <class TAbsoluteObserver>
    void link_absolute_observer(SceneNode& node, const std::shared_ptr<TAbsoluteObserver>& absolute_observer) const {
        node.set_absolute_observer(absolute_observer.get());
        advance_times_.add_advance_time(absolute_observer);
    };
private:
    AdvanceTimes& advance_times_;
};

void LoadScene::operator()(
    const std::string& scene_filename,
    const std::string& script_filename,
    std::string& next_scene_filename,
    RenderingResources& rendering_resources,
    SceneNodeResources& scene_node_resources,
    Players& players,
    Scene& scene,
    PhysicsEngine& physics_engine,
    ButtonPress& button_press,
    KeyBindings& key_bindings,
    SelectedCameras& selected_cameras,
    const CameraConfig& camera_config,
    const PhysicsEngineConfig& physics_engine_config,
    RenderLogics& render_logics,
    RenderLogic& scene_logic,
    ReadPixelsLogic& read_pixels_logic,
    DirtmapLogic& dirtmap_logic,
    SkyboxLogic& skybox_logic,
    GameLogic& game_logic,
    BaseLog& base_log,
    UiFocus& ui_focus,
    SubstitutionString& substitutions,
    size_t& num_renderings,
    std::map<std::string, size_t>& selection_ids,
    bool verbose,
    std::recursive_mutex& mutex,
    const RegexSubstitutionCache& rsc)
{
    std::ifstream ifs{script_filename};
    static const std::regex osm_resource_reg(
        "^(?:\\r?\\n|\\s)*osm_resource\\r?\\n"
        "\\s*name=([\\w+-.]+)\\r?\\n"
        "\\s*filename=([\\w-. \\(\\)/+-]+)\\r?\\n"
        "\\s*heightmap=(#?[\\w-. \\(\\)/+-]*)\\r?\\n"
        "\\s*terrain_texture=(#?[\\w-. \\(\\)/+-]*)\\r?\\n"
        "\\s*dirt_texture=(#?[\\w-. \\(\\)/+-]*)\\r?\\n"
        "\\s*street_crossing_texture=(#?[\\w-. \\(\\)/+-]*)\\r?\\n"
        "\\s*street_texture=(#?[\\w-. \\(\\)/+-]*)\\r?\\n"
        "\\s*path_crossing_texture=(#?[\\w-. \\(\\)/+-]*)\\r?\\n"
        "\\s*path_texture=(#?[\\w-. \\(\\)/+-]*)\\r?\\n"
        "\\s*curb_street_texture=(#?[\\w-. \\(\\)/+-]*)\\r?\\n"
        "\\s*curb_path_texture=(#?[\\w-. \\(\\)/+-]*)\\r?\\n"
        "\\s*facade_textures=([#\\s\\w-. \\(\\)/+-]*)\\r?\\n"
        "\\s*ceiling_texture=(#?[\\w-. \\(\\)/+-]*)\\r?\\n"
        "\\s*barrier_texture=(#?[\\w-. \\(\\)/+-]*)\\r?\\n"
        "\\s*barrier_blend_mode=(off|binary|continuous)\\r?\\n"
        "\\s*roof_texture=([\\w-. \\(\\)/+-]*)\\r?\\n"
        "\\s*tree_resource_names=([,:\\s\\w-. \\(\\)/+-]*)\\r?\\n"
        "\\s*grass_resource_names=([,:\\s\\w-. \\(\\)/+-]*)\\r?\\n"
        "((?:\\s*wayside_resource_names=(?:[,:\\s\\w-. \\(\\)/+-]*)\\r?\\n)*)"
        "\\s*default_street_width=([\\w+-.]+)\\r?\\n"
        "\\s*roof_width=([\\w+-.]+)\\r?\\n"
        "\\s*scale=([\\w+-.]+)\\r?\\n"
        "\\s*uv_scale_terrain=([\\w+-.]+)\\r?\\n"
        "\\s*uv_scale_street=([\\w+-.]+)\\r?\\n"
        "\\s*uv_scale_facade=([\\w+-.]+)\\r?\\n"
        "\\s*uv_scale_ceiling=([\\w+-.]+)\\r?\\n"
        "\\s*uv_scale_barrier_wall=([\\w+-.]+)\\r?\\n"
        "\\s*with_roofs=(0|1)\\r?\\n"
        "\\s*with_ceilings=(0|1)\\r?\\n"
        "\\s*building_bottom=([\\w+-.]+)\\r?\\n"
        "\\s*default_building_top=([\\w+-.]+)\\r?\\n"
        "\\s*default_barrier_top=([\\w+-.]+)\\r?\\n"
        "\\s*remove_backfacing_triangles=(0|1)\\r?\\n"
        "\\s*with_tree_nodes=(0|1)\\r?\\n"
        "\\s*forest_outline_tree_distance=([\\w+-.]+)\\r?\\n"
        "\\s*forest_outline_tree_inwards_distance=([\\w+-.]+)\\r?\\n"
        "\\s*much_grass_distance=([\\w+-.]+)\\r?\\n"
        "\\s*raceway_beacon_distance=([\\w+-.]+)\\r?\\n"
        "\\s*with_terrain=(0|1)\\r?\\n"
        "\\s*with_buildings=(0|1)\\r?\\n"
        "\\s*only_raceways=(0|1)\\r?\\n"
        "\\s*highway_name_pattern=(.*)\\r?\\n"
        "\\s*excluded_highways=(.*)\\r?\\n"
        "\\s*path_tags=(.*)\\r?\\n"
        "\\s*steiner_point_distances_road=([ \\w+-.]*)\\r?\\n"
        "\\s*steiner_point_distances_steiner=([ \\w+-.]*)\\r?\\n"
        "\\s*curb_alpha=([\\w+-.]+)\\r?\\n"
        "\\s*raise_streets_amount=([\\w+-.]+)\\r?\\n"
        "\\s*extrude_curb_amount=([\\w+-.]+)\\r?\\n"
        "\\s*extrude_street_amount=([\\w+-.]+)\\r?\\n"
        "\\s*street_light_resource_names=([,:\\s\\w-. \\(\\)/+-]*)\\r?\\n"
        "\\s*max_wall_width=([\\w+-.]+)\\r?\\n"
        "\\s*with_height_bindings=(0|1)\\r?\\n"
        "\\s*street_node_smoothness=([\\w+-.]+)\\r?\\n"
        "\\s*street_edge_smoothness=([\\w+-.]+)\\r?\\n"
        "\\s*terrain_edge_smoothness=([\\w+-.]+)\\r?\\n"
        "\\s*driving_direction=(center|left|right)$");
    static const std::regex obj_resource_reg(
        "^(?:\\r?\\n|\\s)*obj_resource\\r?\\n"
        "\\s*name=([\\w-. \\(\\)/+-]+)\\r?\\n"
        "\\s*filename=([\\w-. \\(\\)/+-]+)\\r?\\n"
        "\\s*position=([\\w+-.]+) ([\\w+-.]+) ([\\w+-.]+)\\r?\\n"
        "\\s*rotation=([\\w+-.]+) ([\\w+-.]+) ([\\w+-.]+)\\r?\\n"
        "\\s*scale=([\\w+-.]+) ([\\w+-.]+) ([\\w+-.]+)\\r?\\n"
        "\\s*is_small=(0|1)\\r?\\n"
        "\\s*blend_mode=(off|binary|continuous)\\r?\\n"
        "\\s*cull_faces=(0|1)\\r?\\n"
        "\\s*occluded_type=(off|color|depth)\\r?\\n"
        "\\s*occluder_type=(off|white|black)\\r?\\n"
        "\\s*occluded_by_black=(0|1)\\r?\\n"
        "\\s*aggregate_mode=(off|once|sorted|instances_once|instances_sorted)\\r?\\n"
        "\\s*transformation_mode=(all|position|position_lookat)(\\r?\\n"
        "\\s*no_werror)?$");
    static const std::regex gen_triangle_rays_reg("^(?:\\r?\\n|\\s)*gen_triangle_rays name=([\\w+-.]+) npoints=([\\w+-.]+) lengths=([\\w+-.]+) ([\\w+-.]+) ([\\w+-.]+) delete_triangles=(0|1)$");
    static const std::regex gen_ray_reg("^(?:\\r?\\n|\\s)*gen_ray name=([\\w+-.]+) from=([\\w+-.]+) ([\\w+-.]+) ([\\w+-.]+) to=([\\w+-.]+) ([\\w+-.]+) ([\\w+-.]+)$");
    static const std::regex square_resource_reg(
        "^(?:\\r?\\n|\\s)*square_resource\\r?\\n"
        "\\s*name=([\\w+-.]+)\\r?\\n"
        "\\s*texture_filename=(#?[\\w-. \\(\\)/+-]+)\\r?\\n"
        "\\s*min=([\\w+-.]+) ([\\w+-.]+)\\r?\\n"
        "\\s*max=([\\w+-.]+) ([\\w+-.]+)\\r?\\n"
        "\\s*is_small=(0|1)\\r?\\n"
        "\\s*occluded_type=(off|color|depth)\\r?\\n"
        "\\s*occluder_type=(off|white|black)\\r?\\n"
        "\\s*ambience=([\\w+-.]+) ([\\w+-.]+) ([\\w+-.]+)\\r?\\n"
        "\\s*blend_mode=(off|binary|continuous)\\r?\\n"
        "\\s*cull_faces=(0|1)\\r?\\n"
        "\\s*aggregate_mode=(off|once|sorted|instances_once|instances_sorted)\\r?\\n"
        "\\s*transformation_mode=(all|position|position_lookat)$");
    static const std::regex blending_x_resource_reg("^(?:\\r?\\n|\\s)*blending_x_resource name=([\\w+-.]+) texture_filename=([\\w-. \\(\\)/+-]+) min=([\\w+-.]+) ([\\w+-.]+) max=([\\w+-.]+) ([\\w+-.]+)$");
    static const std::regex binary_x_resource_reg("^(?:\\r?\\n|\\s)*binary_x_resource name=([\\w+-.]+) texture_filename=([\\w-. \\(\\)/+-]+) min=([\\w+-.]+) ([\\w+-.]+) max=([\\w+-.]+) ([\\w+-.]+) ambience=([\\w+-.]+) ([\\w+-.]+) ([\\w+-.]+) is_small=(0|1) occluder_type=(off|white|black)$");
    static const std::regex node_instance_reg("^(?:\\r?\\n|\\s)*node_instance parent=([\\w-.<>]+) name=([\\w+-.]+) position=([\\w+-.]+) ([\\w+-.]+) ([\\w+-.]+) rotation=([\\w+-.]+) ([\\w+-.]+) ([\\w+-.]+) scale=([\\w+-.]+)(?: aggregate=(true|false))?$");
    static const std::regex renderable_instance_reg("^(?:\\r?\\n|\\s)*renderable_instance name=([\\w+-.]+) node=([\\w+-.]+) resource=([\\w-. \\(\\)/+-]+)(?: regex=(.*))?$");
    static const std::regex rigid_cuboid_reg("^(?:\\r?\\n|\\s)*rigid_cuboid node=([\\w+-.]+) hitbox=([\\w-. \\(\\)/+-]+)(?: tirelines=([\\w-. \\(\\)/+-]+))? mass=([\\w+-.]+) size=([\\w+-.]+) ([\\w+-.]+) ([\\w+-.]+)(?: com=([\\w+-.]+) ([\\w+-.]+) ([\\w+-.]+))?$");
    static const std::regex gun_reg("^(?:\\r?\\n|\\s)*gun node=([\\w+-.]+) parent_rigid_body_node=([\\w+-.]+) cool-down=([\\w+-.]+) renderable=([\\w-. \\(\\)/+-]+) hitbox=([\\w-. \\(\\)/+-]+) mass=([\\w+-.]+) velocity=([\\w+-.]+) lifetime=([\\w+-.]+) damage=([\\w+-.]+) size=([\\w+-.]+) ([\\w+-.]+) ([\\w+-.]+)$");
    static const std::regex trigger_gun_ai_reg("^(?:\\r?\\n|\\s)*trigger_gun_ai base_shooter_node=([\\w+-.]+) base_target_node=([\\w+-.]+) gun_node=([\\w+-.]+)$");
    static const std::regex damageable_reg("^(?:\\r?\\n|\\s)*damageable node=([\\w+-.]+) health=([\\w+-.]+)$");
    static const std::regex crash_reg("^(?:\\r?\\n|\\s)*crash node=([\\w+-.]+) damage=([\\w+-.]+)$");
    static const std::regex relative_transformer_reg("^(?:\\r?\\n|\\s)*relative_transformer node=([\\w+-.]+)$");
    static const std::regex wheel_reg("^(?:\\r?\\n|\\s)*wheel rigid_body=([\\w+-.]+) node=([\\w+-.]*) position=([\\w+-.]+) ([\\w+-.]+) ([\\w+-.]+) radius=([\\w+-.]+) engine=([\\w+-.]+) break_force=([\\w+-.]+) sKs=([\\w+-.]+) sKa=([\\w+-.]+) pKs=([\\w+-.]+) pKa=([\\w+-.]+) musF=([ \\w+-.]+) musC=([ \\w+-.]+) mufF=([ \\w+-.]+) mufC=([ \\w+-.]+) tire_id=(\\d+)$");
    static const std::regex create_engine_reg("^(?:\\r?\\n|\\s)*create_engine rigid_body=([\\w+-.]+) name=([\\w+-.]+) power=([\\w+-.]+)$");
    static const std::regex player_create_reg(
        "^(?:\\r?\\n|\\s)*player_create"
        "\\s+name=([\\w+-.]+)"
        "\\s+team=([\\w+-.]+)"
        "\\s+mode=(ramming|racing|bystander)"
        "\\s+unstuck_mode=(off|reverse|delete)"
        "\\s+driving_mode=(arena|city)"
        "\\s+driving_direction=(center|left|right)$");
    static const std::regex player_set_node_reg("^(?:\\r?\\n|\\s)*player_set_node player_name=([\\w+-.]+) node=([\\w+-.]+)$");
    static const std::regex player_set_aiming_gun_reg("^(?:\\r?\\n|\\s)*player_set_aiming_gun player-name=([\\w+-.]+) yaw_node=([\\w+-.]+) gun_node=([\\w+-.]*)$");
    static const std::regex player_set_surface_power_reg("^(?:\\r?\\n|\\s)*player_set_surface_power player-name=([\\w+-.]+) forward=([\\w+-.]+) backward=([\\w+-.]*)$");
    static const std::regex player_set_tire_angle_reg("^(?:\\r?\\n|\\s)*player_set_tire_angle player-name=([\\w+-.]+) tire_id=(\\d+) tire_angle_left=([\\w+-.]*) tire_angle_right=([\\w+-.]*)$");
    static const std::regex player_set_waypoint_reg("^(?:\\r?\\n|\\s)*player_set_waypoint player-name=([\\w+-.]+) position=([\\w+-.]*) ([\\w+-.]*)$");
    static const std::regex team_set_waypoint_reg("^(?:\\r?\\n|\\s)*team_set_waypoint team-name=([\\w+-.]+) position=([\\w+-.]*) ([\\w+-.]*)$");
    static const std::regex camera_key_binding_reg("^(?:\\r?\\n|\\s)*camera_key_binding key=([\\w+-.]+) gamepad_button=([\\w+-.]*) joystick_digital_axis=([\\w+-.]*) joystick_digital_axis_sign=([\\w+-.]+)$");
    static const std::regex abs_idle_binding_reg(
        "^(?:\\r?\\n|\\s)*abs_idle_binding\\r?\\n"
        "\\s*node=([\\w+-.]+)\\r?\\n"
        "\\s*tires_z=([\\w+-.]+) ([\\w+-.]+) ([\\w+-.]+)$");
    static const std::regex abs_key_binding_reg(
        "^(?:\\r?\\n|\\s)*abs_key_binding\\r?\\n"
        "\\s*node=([\\w+-.]+)\\r?\\n"
        "\\s*key=([\\w+-.]+)"
        "(?:\\r?\\n\\s*gamepad_button=([\\w+-.]+))?"
        "(?:\\r?\\n\\s*joystick_digital_axis=([\\w+-.]+)\\r?\\n"
        "\\s*joystick_digital_axis_sign=([\\w+-.]+))?"
        "(?:\\r?\\n\\s*force=([\\w+-.]+) ([\\w+-.]+) ([\\w+-.]+)\\r?\\n"
        "\\s*position=([\\w+-.]+) ([\\w+-.]+) ([\\w+-.]+))?"
        "(?:\\r?\\n\\s*rotate=([\\w+-.]+) ([\\w+-.]+) ([\\w+-.]+))?"
        "(?:\\r?\\n\\s*surface_power=([\\w+-.]+))?"
        "(?:\\r?\\n\\s*max_velocity=([\\w+-.]+))?"
        "(?:\\r?\\n\\s*tire_id=(\\d+)\\r?\\n"
        "\\s*tire_angle_velocities=([ \\w+-.]+)\\r?\\n"
        "\\s*tire_angles=([ \\w+-.]+))?"
        "(?:\\r?\\n\\s*tires_z=([\\w+-.]+) ([\\w+-.]+) ([\\w+-.]+))?$");
    static const std::regex rel_key_binding_reg(
        "^(?:\\r?\\n|\\s)*rel_key_binding\\r?\\n"
        "\\s*node=([\\w+-.]+)\\r?\\n"
        "\\s*key=([\\w+-.]+)"
        "(?:\\r?\\n\\s*gamepad_button=([\\w+-.]*))?"
        "\\s*joystick_digital_axis=([\\w+-.]*)\\r?\\n"
        "\\s*joystick_digital_axis_sign=([\\w+-.]+)\\r?\\n"
        "\\s*angular_velocity_press=([\\w+-.]+) ([\\w+-.]+) ([\\w+-.]+)\\r?\\n"
        "\\s*angular_velocity_repeat=([\\w+-.]+) ([\\w+-.]+) ([\\w+-.]+)$");
    static const std::regex gun_key_binding_reg(
        "^(?:\\r?\\n|\\s)*gun_key_binding\\r?\\n"
        "\\s*node=([\\w+-.]+)\\r?\\n"
        "\\s*key=([\\w+-.]+)"
        "(?:\\r?\\n\\s*gamepad_button=([\\w+-.]*))?"
        "(?:\\r?\\n\\s*joystick_digital_axis=([\\w+-.]+)\\r?\\n"
        "\\s*joystick_digital_axis_sign=([\\w+-.]+))?$");
    static const std::regex console_log_reg("^(?:\\r?\\n|\\s)*console_log node=([\\w+-.]+) format=(\\d+)$");
    static const std::regex visual_global_log_reg(
        "^(?:\\r?\\n|\\s)*visual_global_log\\r?\\n"
        "\\s*ttf_file=([\\w-. \\(\\)/+-]+)\\r?\\n"
        "\\s*position=([\\w+-.]+) ([\\w+-.]+)\\r?\\n"
        "\\s*font_height=([\\w+-.]+)\\r?\\n"
        "\\s*line_distance=([\\w+-.]+)\\r?\\n"
        "\\s*nentries=([\\d+]+)\\r?\\n"
        "\\s*severity=(info|critical)$");
    static const std::regex visual_node_status_reg(
        "^(?:\\r?\\n|\\s)*visual_node_status\\r?\\n"
        "\\s*node=([\\w+-.]+)\\r?\\n"
        "\\s*format=(\\d+)\\r?\\n"
        "\\s*ttf_file=([\\w-. \\(\\)/+-]+)\\r?\\n"
        "\\s*position=([\\w+-.]+) ([\\w+-.]+)\\r?\\n"
        "\\s*font_height=([\\w+-.]+)\\r?\\n"
        "\\s*line_distance=([\\w+-.]+)$");
    static const std::regex visual_node_status_3rd_reg(
        "^(?:\\r?\\n|\\s)*visual_node_status_3rd\\r?\\n"
        "\\s*node=([\\w+-.]+)\\r?\\n"
        "\\s*format=(\\d+)\\r?\\n"
        "\\s*ttf_file=([\\w-. \\(\\)/+-]+)\\r?\\n"
        "\\s*offset=([\\w+-.]+) ([\\w+-.]+)\\r?\\n"
        "\\s*font_height=([\\w+-.]+)\\r?\\n"
        "\\s*line_distance=([\\w+-.]+)$");
    static const std::regex loading_reg(
        "^(?:\\r?\\n|\\s)*loading"
        "\\s*ttf_file=([\\w-. \\(\\)/+-]+)\\r?\\n"
        "\\s*position=([\\w+-.]+) ([\\w+-.]+)\\r?\\n"
        "\\s*font_height=([\\w+-.]+)\\r?\\n"
        "\\s*line_distance=([\\w+-.]+)\\r?\\n"
        "\\s*text=(.*)$");
    static const std::regex countdown_reg(
        "^(?:\\r?\\n|\\s)*countdown"
        "\\s*ttf_file=([\\w-. \\(\\)/+-]+)\\r?\\n"
        "\\s*position=([\\w+-.]+) ([\\w+-.]+)\\r?\\n"
        "\\s*font_height=([\\w+-.]+)\\r?\\n"
        "\\s*line_distance=([\\w+-.]+)\\r?\\n"
        "\\s*nseconds=([\\w+-.]+)$");
    static const std::regex players_stats_reg(
        "^(?:\\r?\\n|\\s)*players_stats\\r?\\n"
        "\\s*ttf_file=([\\w-. \\(\\)/+-]+)\\r?\\n"
        "\\s*position=([\\w+-.]+) ([\\w+-.]+)\\r?\\n"
        "\\s*font_height=([\\w+-.]+)\\r?\\n"
        "\\s*line_distance=([\\w+-.]+)$");
    static const std::regex scene_selector_reg(
        "^(?:\\r?\\n|\\s)*scene_selector\\r?\\n"
        "\\s*id=([\\w+-.]+)\\r?\\n"
        "\\s*ttf_file=([\\w-. \\(\\)/+-]+)\\r?\\n"
        "\\s*position=([\\w+-.]+) ([\\w+-.]+)\\r?\\n"
        "\\s*font_height=([\\w+-.]+)\\r?\\n"
        "\\s*line_distance=([\\w+-.]+)\\r?\\n"
        "\\s*scene_files=([\\r\\n\\w-. \\(\\)/+-:=]+)$");
    static const std::regex clear_parameters_reg(
        "^(?:\\r?\\n|\\s)*clear_parameters$");
    static const std::regex parameter_setter_reg(
        "^(?:\\r?\\n|\\s)*parameter_setter\\r?\\n"
        "\\s*id=([\\w+-.]+)\\r?\\n"
        "\\s*ttf_file=([\\w-. \\(\\)/+-]+)\\r?\\n"
        "\\s*position=([\\w+-.]+) ([\\w+-.]+)\\r?\\n"
        "\\s*font_height=([\\w+-.]+)\\r?\\n"
        "\\s*line_distance=([\\w+-.]+)\\r?\\n"
        "\\s*default=([\\d]+)\\r?\\n"
        "\\s*parameters=([\\r\\n\\w-. \\(\\)/+-:=]+)$");
    static const std::regex set_renderable_style_reg(
        "^(?:\\r?\\n|\\s)*set_renderable_style\\r?\\n"
        "\\s*selector=([\\w+-.]+)\\r?\\n"
        "\\s*node=([\\w+-.]+)\\r?\\n"
        "\\s*ambience=([\\w+-.]+) ([\\w+-.]+) ([\\w+-.]+)\\r?\\n"
        "\\s*diffusivity=([\\w+-.]+) ([\\w+-.]+) ([\\w+-.]+)\\r?\\n"
        "\\s*specularity=([\\w+-.]+) ([\\w+-.]+) ([\\w+-.]+)$");
    static const std::regex ui_background_reg("^(?:\\r?\\n|\\s)*ui_background texture=([\\w-. \\(\\)/+-]+) target_focus=(menu|loading|countdown|scene)$");
    static const std::regex hud_image_reg("^(?:\\r?\\n|\\s)*hud_image node=([\\w+-.]+) filename=([\\w-. \\(\\)/+-]+) center=([\\w+-.]+) ([\\w+-.]+) size=([\\w+-.]+) ([\\w+-.]+)$");
    static const std::regex perspective_camera_reg("^(?:\\r?\\n|\\s)*perspective_camera node=([\\w+-.]+) y-fov=([\\w+-.]+) near_plane=([\\w+-.]+) far_plane=([\\w+-.]+) requires_postprocessing=(0|1)$");
    static const std::regex ortho_camera_reg("^(?:\\r?\\n|\\s)*ortho_camera node=([\\w+-.]+) near_plane=([\\w+-.]+) far_plane=([\\w+-.]+) left_plane=([\\w+-.]+) right_plane=([\\w+-.]+) bottom_plane=([\\w+-.]+) top_plane=([\\w+-.]+) requires_postprocessing=(0|1)$");
    static const std::regex light_reg("^(?:\\r?\\n|\\s)*light node=([\\w+-.]+) black_node=([\\w+-.]*) update=(once|always) with_depth_texture=(0|1) ambience=([\\w+-.]+) ([\\w+-.]+) ([\\w+-.]+) diffusivity=([\\w+-.]+) ([\\w+-.]+) ([\\w+-.]+) specularity=([\\w+-.]+) ([\\w+-.]+) ([\\w+-.]+) shadow=(0|1)$");
    static const std::regex look_at_node_reg("^(?:\\r?\\n|\\s)*look_at_node follower=([\\w+-.]+) followed=([\\w+-.]+)$");
    static const std::regex keep_offset_reg("^(?:\\r?\\n|\\s)*keep_offset follower=([\\w+-.]+) followed=([\\w+-.]+) offset=([\\w+-.]+) ([\\w+-.]+) ([\\w+-.]+)$");
    static const std::regex yaw_pitch_look_at_nodes_reg("^(?:\\r?\\n|\\s)*yaw_pitch_look_at_nodes yaw_node=([\\w+-.]+) pitch_node=([\\w+-.]+) parent_follower_rigid_body_node=([\\w+-.]+) followed=([\\w+-.]*) bullet_start_offset=([\\w+-.]+) bullet_velocity=([\\w+-.]+) gravity=([\\w+-.]+)$");
    static const std::regex follow_node_reg(
        "^(?:\\r?\\n|\\s)*follow_node\\r?\\n"
        "\\s*follower=([\\w+-.]+)\\r?\\n"
        "\\s*followed=([\\w+-.]+)\\r?\\n"
        "\\s*distance=([\\w+-.]+)\\r?\\n"
        "\\s*node_displacement=([\\w+-.]+) ([\\w+-.]+) ([\\w+-.]+)\\r?\\n"
        "\\s*look_at_displacement=([\\w+-.]+) ([\\w+-.]+) ([\\w+-.]+)\\r?\\n"
        "\\s*snappiness=([\\w+-.]+)\\r?\\n"
        "\\s*y_adaptivity=([\\w+-.]+)\\r?\\n"
        "\\s*y_snappiness=([\\w+-.]+)$");
    static const std::regex add_texture_descriptor_reg(
        "^(?:\\r?\\n|\\s)*add_texture_descriptor\\r?\\n"
        "\\s*name=([\\w+-.]+)\\r?\\n"
        "\\s*color=([\\w-. \\(\\)/+-]+)"
        "\\s*normal=([\\w-. \\(\\)/+-]*)"
        "\\s*color_mode=(rgb|rgba)"
        "\\s*histogram=([\\w-. \\(\\)/+-]*)"
        "\\s*mixed=([\\w-. \\(\\)/+-]*)"
        "\\s*overlap_npixels=(\\d+)"
        "\\s*mean_color=([\\w+-.]+) ([\\w+-.]+) ([\\w+-.]+)$");
    static const std::regex record_track_reg("^(?:\\r?\\n|\\s)*record_track node=([\\w+-.]+) filename=([\\w-. \\(\\)/+-]+)$");
    static const std::regex playback_track_reg("^(?:\\r?\\n|\\s)*playback_track node=([\\w+-.]+) speed=([\\w+-.]+) filename=([\\w-. \\(\\)/+-]+)$");
    static const std::regex check_points_reg(
        "^(?:\\r?\\n|\\s)*check_points\\r?\\n"
        "\\s*moving_node=([\\w+-.]+)\\r?\\n"
        "\\s*resource=([\\w-. \\(\\)/+-]+)\\r?\\n"
        "\\s*player=([\\w+-.]+)\\r?\\n"
        "\\s*nbeacons=(\\d+)\\r?\\n"
        "\\s*nth=(\\d+)\\r?\\n"
        "\\s*nahead=(\\d+)\\r?\\n"
        "\\s*radius=([\\w+-.]+)\\r?\\n"
        "\\s*track_filename=([\\w-. \\(\\)/+-]+)$");
    static const std::regex set_camera_cycle_reg("^(?:\\r?\\n|\\s)*set_camera_cycle name=(near|far)((?: [\\w+-.]+)*)$");
    static const std::regex set_camera_reg("^(?:\\r?\\n|\\s)*set_camera ([\\w+-.]+)$");
    static const std::regex set_dirtmap_reg("^(?:\\r?\\n|\\s)*set_dirtmap filename=([\\w-. \\(\\)/+-]+) discreteness=([\\w+-.]+) wrap_mode=(repeat|clamp_to_edge|clamp_to_border)$");
    static const std::regex set_skybox_reg("^(?:\\r?\\n|\\s)*set_skybox alias=([\\w+-.]+) filenames=([\\w-. \\(\\)/+-]+) ([\\w-. \\(\\)/+-]+) ([\\w-. \\(\\)/+-]+) ([\\w-. \\(\\)/+-]+) ([\\w-. \\(\\)/+-]+) ([\\w-. \\(\\)/+-]+)$");
    static const std::regex set_preferred_car_spawner_reg("^(?:\\r?\\n|\\s)*set_preferred_car_spawner player=([\\w+-.]+)\\s+macro=([\\w]+)\\s+parameters=([: \\w+-.]*)$");
    static const std::regex set_vip_reg("^(?:\\r?\\n|\\s)*set_vip player=([\\w+-.]+)$");
    static const std::regex burn_in_reg("^(?:\\r?\\n|\\s)*burn_in seconds=([\\w+-.]+)$");
    static const std::regex append_focus_reg("^(?:\\r?\\n|\\s)*append_focus (menu|loading|countdown|scene)$");
    static const std::regex wayside_resource_names_reg(
        "(?:\\s*wayside_resource_names=\\r?\\n"
        "\\s*min_dist:([\\w+-.]+)\\r?\\n"
        "\\s*max_dist:([\\w+-.]+)\\r?\\n"
        "([\\s,:\\w-. \\(\\)/+-]*)\\r?\\n|(.+))");
    static const std::regex set_spawn_points_reg("^(?:\\r?\\n|\\s)*set_spawn_points node=([\\w+-.]+) resource=([\\w+-.]+)$");
    static const std::regex set_way_points_reg("^(?:\\r?\\n|\\s)*set_way_points player=([\\w+-.]+)\\s+node=([\\w+-.]+) resource=([\\w+-.]+)$");

    Linker linker{physics_engine.advance_times_};

    MacroFileExecutor::UserFunction execute_user_function = [&, linker](
        const std::function<std::string(const std::string&)>& fpath,
        const MacroLineExecutor& macro_line_executor,
        const std::string& line) -> bool
    {
        std::smatch match;
        if (std::regex_match(line, match, osm_resource_reg)) {
            std::list<WaysideResourceNames> waysides;
            findall(
                match[19].str(),
                wayside_resource_names_reg,
                [&waysides, &match](const std::smatch& m){
                    if (!m[4].str().empty()) {
                        throw std::runtime_error("Could not parse \"" + match[18].str() + "\", unknown element: \"" + m[4].str() + '"');
                    }
                    waysides.push_back(WaysideResourceNames{
                        .min_dist = safe_stof(m[1].str()),
                        .max_dist = safe_stof(m[2].str()),
                        .resource_names = string_to_vector(m[3].str())});
                    });
            scene_node_resources.add_resource(
                match[1].str(),                                                   // name
                std::make_shared<RenderableOsmMap>(
                    scene_node_resources,
                    rendering_resources,
                    fpath(match[2].str()),                                        // filename
                    fpath(match[3].str()),                                        // heightmap
                    fpath(match[4].str()),                                        // terrain_texture
                    fpath(match[5].str()),                                        // dirt_texture
                    fpath(match[6].str()),                                        // street_crossing_texture
                    fpath(match[7].str()),                                        // street_texture
                    fpath(match[8].str()),                                        // path_crossing_texture
                    fpath(match[9].str()),                                        // path_texture
                    fpath(match[10].str()),                                        // curb_street_texture
                    fpath(match[11].str()),                                       // curb_path_texture
                    string_to_vector(match[12].str(), fpath),                     // facade_textures
                    fpath(match[13].str()),                                       // ceiling_texture
                    fpath(match[14].str()),                                       // barrier_texture
                    blend_mode_from_string(match[15].str()),                      // barrier_blend_mode
                    fpath(match[16].str()),                                       // roof_texture
                    string_to_vector(match[17].str()),                            // tree_resource_names
                    string_to_vector(match[18].str()),                            // grass_resource_names
                    waysides,                                                     // wayside_resource_names
                    safe_stof(match[20].str()),                                   // default_street_width
                    safe_stof(match[21].str()),                                   // roof_width
                    safe_stof(match[22].str()),                                   // scale
                    safe_stof(match[23].str()),                                   // uv_scale_terrain
                    safe_stof(match[24].str()),                                   // uv_scale_street
                    safe_stof(match[25].str()),                                   // uv_scale_facade
                    safe_stof(match[26].str()),                                   // uv_scale_ceiling
                    safe_stof(match[27].str()),                                   // uv_scale_barrier_wall
                    safe_stob(match[28].str()),                                   // with_roofs
                    safe_stob(match[29].str()),                                   // with_ceilings
                    safe_stof(match[30].str()),                                   // building_bottom
                    safe_stof(match[31].str()),                                   // default_building_top
                    safe_stof(match[32].str()),                                   // default_barrier_top
                    safe_stob(match[33].str()),                                   // remove_backfacing_triangles
                    safe_stob(match[34].str()),                                   // with_tree_nodes
                    safe_stof(match[35].str()),                                   // forest_outline_tree_distance
                    safe_stof(match[36].str()),                                   // forest_outline_tree_inwards_distance
                    safe_stof(match[37].str()),                                   // much_grass_distance
                    safe_stof(match[38].str()),                                   // raceway_beacon_distance
                    safe_stob(match[39].str()),                                   // with_terrain
                    safe_stob(match[40].str()),                                   // with_buildings
                    safe_stob(match[41].str()),                                   // only_raceways
                    match[42].str(),                                              // highway_name_pattern
                    string_to_set(match[43].str()),                               // excluded_highways
                    string_to_set(match[44].str()),                               // path_tags
                    string_to_vector(match[45].str(), safe_stof),                 // steiner_point_distances_road
                    string_to_vector(match[46].str(), safe_stof),                 // steiner_point_distances_steiner
                    safe_stof(match[47].str()),                                   // curb_alpha
                    safe_stof(match[48].str()),                                   // raise_streets_amount
                    safe_stof(match[49].str()),                                   // extrude_curb_amount
                    safe_stof(match[50].str()),                                   // extrude_street_amount
                    string_to_vector(match[51].str()),                            // street_light_resource_names
                    safe_stof(match[52].str()),                                   // max_wall_width
                    safe_stob(match[53].str()),                                   // with_height_bindings
                    safe_stof(match[54].str()),                                   // street_node_smoothness
                    safe_stof(match[55].str()),                                   // street_edge_smoothness
                    safe_stof(match[56].str()),                                   // terrain_edge_smoothness
                    driving_direction_from_string(match[57].str())));             // driving_direction
        } else if (std::regex_match(line, match, obj_resource_reg)) {
            scene_node_resources.add_resource(match[1].str(), std::make_shared<RenderableObjFile>(
                fpath(match[2].str()),
                LoadMeshConfig{
                    .position = FixedArray<float, 3>{
                        safe_stof(match[3].str()),
                        safe_stof(match[4].str()),
                        safe_stof(match[5].str())},
                    .rotation = FixedArray<float, 3>{
                        safe_stof(match[6].str()) / 180 * float(M_PI),
                        safe_stof(match[7].str()) / 180 * float(M_PI),
                        safe_stof(match[8].str()) / 180 * float(M_PI)},
                    .scale = FixedArray<float, 3>{
                        safe_stof(match[9].str()),
                        safe_stof(match[10].str()),
                        safe_stof(match[11].str())},
                    .is_small = safe_stob(match[12].str()),
                    .blend_mode = blend_mode_from_string(match[13].str()),
                    .cull_faces = safe_stob(match[14].str()),
                    .occluded_type = occluded_type_from_string(match[15].str()),
                    .occluder_type = occluder_type_from_string(match[16].str()),
                    .occluded_by_black = safe_stob(match[17].str()),
                    .aggregate_mode = aggregate_mode_from_string(match[18].str()),
                    .transformation_mode = transformation_mode_from_string(match[19].str()),
                    .apply_static_lighting = false,
                    .werror = match[20].str() == ""},
                rendering_resources));
        } else if (std::regex_match(line, match, gen_triangle_rays_reg)) {
            scene_node_resources.generate_triangle_rays(
                match[1].str(),
                safe_stoi(match[2].str()),
                {
                    safe_stof(match[3].str()),
                    safe_stof(match[4].str()),
                    safe_stof(match[5].str())
                },
                safe_stoi(match[6].str()));
        } else if (std::regex_match(line, match, gen_ray_reg)) {
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
        } else if (std::regex_match(line, match, square_resource_reg)) {
            // 1: name
            // 2: texture_filename
            // 3, 4: min
            // 5, 6: max
            // 7: is_small
            // 8: occluded_type
            // 9: occluder_type
            // 10, 11, 12: ambience
            // 13: blend_mode
            // 14: cull_faces
            // 15: aggregate_mode
            // 16: transformation_mode
            scene_node_resources.add_resource(match[1].str(), std::make_shared<RenderableSquare>(
                FixedArray<float, 2, 2>{
                    safe_stof(match[3].str()), safe_stof(match[4].str()),
                    safe_stof(match[5].str()), safe_stof(match[6].str())},
                Material{
                    .texture_descriptor = {color: fpath(match[2].str())},
                    .occluded_type =  occluded_type_from_string(match[8].str()),
                    .occluder_type = occluder_type_from_string(match[9].str()),
                    .blend_mode = blend_mode_from_string(match[13].str()),
                    .wrap_mode_s = WrapMode::CLAMP_TO_EDGE,
                    .wrap_mode_t = WrapMode::CLAMP_TO_EDGE,
                    .collide = false,
                    .aggregate_mode = aggregate_mode_from_string(match[15].str()),
                    .transformation_mode = transformation_mode_from_string(match[16].str()),
                    .is_small = safe_stob(match[7].str()),
                    .cull_faces = safe_stob(match[14].str()),
                    .ambience = {
                        safe_stof(match[10].str()),
                        safe_stof(match[11].str()),
                        safe_stof(match[12].str())},
                    .diffusivity = {0, 0, 0},
                    .specularity = {0, 0, 0}}.compute_color_mode(),
                rendering_resources));
        } else if (std::regex_match(line, match, blending_x_resource_reg)) {
            scene_node_resources.add_resource(match[1].str(), std::make_shared<RenderableBlendingX>(
                FixedArray<float, 2, 2>{
                    safe_stof(match[3].str()), safe_stof(match[4].str()),
                    safe_stof(match[5].str()), safe_stof(match[6].str())},
                fpath(match[2].str()),
                rendering_resources));
        } else if (std::regex_match(line, match, binary_x_resource_reg)) {
            scene_node_resources.add_resource(match[1].str(), std::make_shared<RenderableBinaryX>(
                FixedArray<float, 2, 2>{
                    safe_stof(match[3].str()), safe_stof(match[4].str()),
                    safe_stof(match[5].str()), safe_stof(match[6].str())},
                fpath(match[2].str()),
                rendering_resources,
                safe_stob(match[10].str()),
                occluder_type_from_string(match[11].str()),
                FixedArray<float, 3>{
                    safe_stof(match[7].str()),
                    safe_stof(match[8].str()),
                    safe_stof(match[9].str())}));
        } else if (std::regex_match(line, match, node_instance_reg)) {
            auto node = new SceneNode(&scene);
            node->set_position(FixedArray<float, 3>{
                safe_stof(match[3].str()),
                safe_stof(match[4].str()),
                safe_stof(match[5].str())});
            node->set_rotation(FixedArray<float, 3>{
                safe_stof(match[6].str()) / 180 * float(M_PI),
                safe_stof(match[7].str()) / 180 * float(M_PI),
                safe_stof(match[8].str()) / 180 * float(M_PI)});
            node->set_scale(
                safe_stof(match[9].str()));
            bool aggregate = (match[10].str() == "true");
            if (match[1].str() == "<dynamic-root>") {
                if (aggregate) {
                    scene.add_root_aggregate_node(match[2].str(), node);
                } else {
                    scene.add_root_node(match[2].str(), node);
                }
            } else if (match[1].str() == "<static-root>") {
                scene.add_static_root_node(match[2].str(), node);
            } else {
                auto parent = scene.get_node(match[1].str());
                node->set_parent(parent);
                if (aggregate) {
                    parent->add_aggregate_child(match[2].str(), node, true);  // true=is_registered
                } else {
                    parent->add_child(match[2].str(), node, true);  // true=is_registered
                }
                scene.register_node(match[2].str(), node);
            }
        } else if (std::regex_match(line, match, renderable_instance_reg)) {
            auto node = scene.get_node(match[2].str());
            scene_node_resources.instantiate_renderable(
                match[3].str(),
                match[1].str(),
                *node,
                {regex: std::regex{match[4].str()}});
        } else if (std::regex_match(line, match, rigid_cuboid_reg)) {
            std::shared_ptr<RigidBody> rb = rigid_cuboid(
                physics_engine.rigid_bodies_,
                safe_stof(match[4].str()),
                FixedArray<float, 3>{
                    safe_stof(match[5].str()),
                    safe_stof(match[6].str()),
                    safe_stof(match[7].str())},
                FixedArray<float, 3>{
                    match[8].str().empty() ? 0.f : safe_stof(match[8].str()),
                    match[9].str().empty() ? 0.f : safe_stof(match[9].str()),
                    match[10].str().empty() ? 0.f : safe_stof(match[10].str())});
            std::list<std::shared_ptr<ColoredVertexArray>> hitbox = scene_node_resources.get_triangle_meshes(match[2].str());
            std::list<std::shared_ptr<ColoredVertexArray>> tirelines;
            if (!match[3].str().empty()) {
                tirelines = scene_node_resources.get_triangle_meshes(match[3].str());
            }
            // 1. Set movable, which updates the transformation-matrix
            scene.get_node(match[1].str())->set_absolute_movable(rb.get());
            // 2. Add to physics engine
            physics_engine.rigid_bodies_.add_rigid_body(rb, hitbox, tirelines);
        } else if (std::regex_match(line, match, gun_reg)) {
            auto parent_rb_node = scene.get_node(match[2].str());
            auto rb = dynamic_cast<RigidBody*>(parent_rb_node->get_absolute_movable());
            if (rb == nullptr) {
                throw std::runtime_error("Absolute movable is not a rigid body");
            }
            std::shared_ptr<Gun> gun = std::make_shared<Gun>(
                scene,                              // scene
                scene_node_resources,               // scene_node_resources
                physics_engine.rigid_bodies_,       // rigid_bodies
                physics_engine.advance_times_,      // advance_times
                safe_stof(match[3].str()),          // cool-down
                rb->rbi_,                           // parent_rigid_body_node
                match[4].str(),                     // bullet-renderable-resource-name
                match[5].str(),                     // bullet-hitbox-resource-name
                safe_stof(match[6].str()),          // bullet-mass
                safe_stof(match[7].str()),          // bullet_velocity
                safe_stof(match[8].str()),          // bullet-lifetime
                safe_stof(match[9].str()),          // bullet-damage
                FixedArray<float, 3>{               // bullet-size
                    safe_stof(match[10].str()),     // bullet-size-x
                    safe_stof(match[11].str()),     // bullet-size-y
                    safe_stof(match[12].str())});   // bullet-size-z
            linker.link_absolute_observer(*scene.get_node(match[1].str()), gun);
        } else if (std::regex_match(line, match, trigger_gun_ai_reg)) {
            auto base_shooter_node = scene.get_node(match[1].str());
            auto rb_shooter = dynamic_cast<RigidBody*>(base_shooter_node->get_absolute_movable());
            if (rb_shooter == nullptr) {
                throw std::runtime_error("Absolute movable is not a rigid body");
            }
            auto base_target_node = scene.get_node(match[2].str());
            auto rb_target = dynamic_cast<RigidBody*>(base_target_node->get_absolute_movable());
            if (rb_target == nullptr) {
                throw std::runtime_error("Absolute movable is not a rigid body");
            }
            auto gun_node = scene.get_node(match[3].str());
            auto gun = dynamic_cast<Gun*>(gun_node->get_absolute_observer());
            if (gun == nullptr) {
                throw std::runtime_error("Absolute observer is not a gun");
            }
            std::shared_ptr<TriggerGunAi> trigger_gun_ai = std::make_shared<TriggerGunAi>(
                *base_shooter_node,
                *base_target_node,
                *gun_node,
                rb_shooter->rbi_,
                rb_target->rbi_,
                physics_engine,
                *gun);
            physics_engine.advance_times_.add_advance_time(trigger_gun_ai);
        } else if (std::regex_match(line, match, damageable_reg)) {
            auto rb = dynamic_cast<RigidBody*>(scene.get_node(match[1].str())->get_absolute_movable());
            if (rb == nullptr) {
                throw std::runtime_error("Absolute movable is not a rigid body");
            }
            auto d = std::make_shared<DeletingDamageable>(
                scene,
                physics_engine.advance_times_,
                match[1].str(),
                safe_stof(match[2].str()),
                mutex);
            if (rb->damageable_ != nullptr) {
                throw std::runtime_error("Rigid body already has a damageable");
            }
            rb->damageable_ = d.get();
            physics_engine.advance_times_.add_advance_time(d);
        } else if (std::regex_match(line, match, crash_reg)) {
            auto rb = dynamic_cast<RigidBody*>(scene.get_node(match[1].str())->get_absolute_movable());
            if (rb == nullptr) {
                throw std::runtime_error("Absolute movable is not a rigid body");
            }
            auto d = std::make_shared<Crash>(
                *rb,
                safe_stof(match[2].str()));  // damage
            rb->collision_observers_.push_back(d);
        } else if (std::regex_match(line, match, relative_transformer_reg)) {
            std::shared_ptr<RelativeTransformer> rt = std::make_shared<RelativeTransformer>(physics_engine.advance_times_);
            linker.link_relative_movable(*scene.get_node(match[1].str()), rt);
        } else if (std::regex_match(line, match, wheel_reg)) {
            std::string rigid_body = match[1].str();
            std::string node = match[2].str();
            FixedArray<float, 3> position{
                safe_stof(match[3].str()),
                safe_stof(match[4].str()),
                safe_stof(match[5].str())};
            float radius = safe_stof(match[6].str());
            std::string engine = match[7].str();
            float break_force = safe_stof(match[8].str());
            float sKs = safe_stof(match[9].str());
            float sKa = safe_stof(match[10].str());
            float pKs = safe_stof(match[11].str());
            float pKa = safe_stof(match[12].str());
            Interp<float> mus{string_to_vector(match[13].str(), safe_stof), string_to_vector(match[14].str(), safe_stof), OutOfRangeBehavior::CLAMP};
            Interp<float> muk{string_to_vector(match[15].str(), safe_stof), string_to_vector(match[16].str(), safe_stof), OutOfRangeBehavior::CLAMP};
            size_t tire_id = safe_stoi(match[17].str());

            auto rb = dynamic_cast<RigidBody*>(scene.get_node(rigid_body)->get_absolute_movable());
            if (rb == nullptr) {
                throw std::runtime_error("Absolute movable is not a rigid body");
            }
            if (!node.empty()) {
                std::shared_ptr<Wheel> wheel = std::make_shared<Wheel>(
                    *rb,
                    physics_engine.advance_times_,
                    tire_id,
                    radius,
                    physics_engine_config.physics_type,
                    physics_engine_config.resolve_collision_type);
                linker.link_relative_movable(*scene.get_node(node), wheel);
            }
            {
                if (auto ep = rb->engines_.find(engine); ep == rb->engines_.end()) {
                    throw std::runtime_error("Could not find engine with name " + engine);
                } else {
                    ep->second.increment_ntires();
                }
                // From: https://www.nanolounge.de/21977/federkonstante-und-masse-bei-auto
                // Ds = 1000 / 4 * 9.8 / 0.02 = 122500 = 1.225e5

                // Da * 1 = 1000 / 4 * 9.8 => Da = 1e4 / 4
                size_t nsprings_tracking = 1;
                float max_dist = 0.3;
                auto tp = rb->tires_.insert({
                    tire_id,
                    Tire{
                        engine,
                        break_force,
                        sKs,
                        sKa,
                        mus,
                        muk,
                        ShockAbsorber{pKs, pKa},
                        TrackingWheel{
                            {1.f, 0.f, 0.f},
                            radius,
                            nsprings_tracking,
                            max_dist,
                            physics_engine_config.dt / physics_engine_config.oversampling},
                        CombinedMagicFormula<float>{
                            .f = FixedArray<MagicFormulaArgmax<float>, 2>{
                                MagicFormulaArgmax<float>{MagicFormula<float>{.B = 41.f * 0.044f * physics_engine_config.longitudinal_friction_steepness}},
                                MagicFormulaArgmax<float>{MagicFormula<float>{.B = 41.f * 0.044f * physics_engine_config.lateral_friction_steepness}}
                            }
                        },
                        position,
                        radius}});
                if (!tp.second) {
                    throw std::runtime_error("Tire with ID \"" + std::to_string(tire_id) + "\" already exists");
                }
            }
        } else if (std::regex_match(line, match, create_engine_reg)) {
            auto rb = dynamic_cast<RigidBody*>(scene.get_node(match[1].str())->get_absolute_movable());
            if (rb == nullptr) {
                throw std::runtime_error("Absolute movable is not a rigid body");
            }
            auto ep = rb->engines_.insert({
                match[2].str(),
                RigidBodyEngine{safe_stof(match[3].str())}});
            if (!ep.second) {
                throw std::runtime_error("Engine with name \"" + match[2].str() + "\" already exists");
            }
        } else if (std::regex_match(line, match, player_create_reg)) {
            auto player = std::make_shared<Player>(
                scene,
                physics_engine.collision_query_,
                players,
                match[1].str(),
                match[2].str(),
                game_mode_from_string(match[3].str()),
                unstuck_mode_from_string(match[4].str()),
                driving_modes.at(match[5].str()),
                driving_direction_from_string(match[6].str()),
                mutex);
            players.add_player(*player);
            physics_engine.advance_times_.add_advance_time(player);
            physics_engine.add_external_force_provider(player.get());
        } else if (std::regex_match(line, match, player_set_node_reg)) {
            auto node = scene.get_node(match[2].str());
            auto rb = dynamic_cast<RigidBody*>(node->get_absolute_movable());
            if (rb == nullptr) {
                throw std::runtime_error("Follower movable is not a rigid body");
            }
            players.get_player(match[1].str()).set_rigid_body(match[2].str(), *node, *rb);
        } else if (std::regex_match(line, match, player_set_aiming_gun_reg)) {
            auto ypln_node = scene.get_node(match[2].str());
            auto ypln = dynamic_cast<YawPitchLookAtNodes*>(ypln_node->get_relative_movable());
            if (ypln == nullptr) {
                throw std::runtime_error("Relative movable is not a ypln");
            }
            Gun* gun = nullptr;
            if (!match[3].str().empty()) {
                auto gun_node = scene.get_node(match[3].str());
                gun = dynamic_cast<Gun*>(gun_node->get_absolute_observer());
                if (gun == nullptr) {
                    throw std::runtime_error("Absolute observer is not a gun");
                }
            }
            players.get_player(match[1].str()).set_ypln(*ypln, gun);
        } else if (std::regex_match(line, match, player_set_surface_power_reg)) {
            players.get_player(match[1].str()).set_surface_power(
                safe_stof(match[2].str()),
                safe_stof(match[3].str()));
        } else if (std::regex_match(line, match, player_set_tire_angle_reg)) {
            players.get_player(match[1].str()).set_tire_angle_y(
                safe_stoi(match[2].str()),
                M_PI / 180.f * safe_stof(match[3].str()),
                M_PI / 180.f * safe_stof(match[4].str()));
        } else if (std::regex_match(line, match, player_set_waypoint_reg)) {
            players.get_player(match[1].str()).set_waypoint({
                safe_stof(match[2].str()),
                safe_stof(match[3].str())});
        } else if (std::regex_match(line, match, team_set_waypoint_reg)) {
            players.set_team_waypoint(
                match[1].str(), {
                    safe_stof(match[2].str()),
                    safe_stof(match[3].str())});
        } else if (std::regex_match(line, match, camera_key_binding_reg)) {
            key_bindings.add_camera_key_binding(CameraKeyBinding{
                base: {
                    key: match[1].str(),
                    gamepad_button: match[2].str(),
                    joystick_axis: match[3].str(),
                    joystick_axis_sign: safe_stof(match[4].str())}});
        } else if (std::regex_match(line, match, abs_idle_binding_reg)) {
            key_bindings.add_absolute_movable_idle_binding(AbsoluteMovableIdleBinding{
                node: scene.get_node(match[1].str()),
                tires_z: {
                    match[2].str().empty() ? 0 : safe_stof(match[2].str()),
                    match[3].str().empty() ? 0 : safe_stof(match[3].str()),
                    match[4].str().empty() ? 1 : safe_stof(match[4].str())}});
        } else if (std::regex_match(line, match, abs_key_binding_reg)) {
            key_bindings.add_absolute_movable_key_binding(AbsoluteMovableKeyBinding{
                base_key: {
                    key: match[2].str(),
                    gamepad_button: match[3].str(),
                    joystick_axis: match[4].str(),
                    joystick_axis_sign: match[5].str().empty() ? 0 : safe_stof(match[5].str())},
                node: scene.get_node(match[1].str()),
                force: {
                    vector: {
                        match[6].str().empty() ? 0 : safe_stof(match[6].str()),
                        match[7].str().empty() ? 0 : safe_stof(match[7].str()),
                        match[8].str().empty() ? 0 : safe_stof(match[8].str())},
                    position: {
                        match[9].str().empty() ? 0 : safe_stof(match[9].str()),
                        match[10].str().empty() ? 0 : safe_stof(match[10].str()),
                        match[11].str().empty() ? 0 : safe_stof(match[11].str())}},
                rotate: {
                    match[12].str().empty() ? 0 : safe_stof(match[12].str()),
                    match[13].str().empty() ? 0 : safe_stof(match[13].str()),
                    match[14].str().empty() ? 0 : safe_stof(match[14].str())},
                surface_power: match[15].str().empty() ? 0 : safe_stof(match[15].str()),
                max_velocity: match[16].str().empty() ? INFINITY : safe_stof(match[16].str()),
                tire_id: match[17].str().empty() ? SIZE_MAX : safe_stoi(match[17].str()),
                tire_angle_interp: Interp<float>{
                    string_to_vector(match[18].str(), safe_stof),
                    string_to_vector(match[19].str(), safe_stof),
                    OutOfRangeBehavior::CLAMP},
                tires_z: {
                    match[20].str().empty() ? 0 : safe_stof(match[20].str()),
                    match[21].str().empty() ? 0 : safe_stof(match[21].str()),
                    match[22].str().empty() ? 0 : safe_stof(match[22].str())}});
        } else if (std::regex_match(line, match, rel_key_binding_reg)) {
            key_bindings.add_relative_movable_key_binding(RelativeMovableKeyBinding{
                base_key: {
                    key: match[2].str(),
                    gamepad_button: match[3].str(),
                    joystick_axis: match[4].str(),
                    joystick_axis_sign: safe_stof(match[5].str())},
                node: scene.get_node(match[1].str()),
                angular_velocity_press: {
                    safe_stof(match[6].str()),
                    safe_stof(match[7].str()),
                    safe_stof(match[8].str())},
                angular_velocity_repeat: {
                    safe_stof(match[9].str()),
                    safe_stof(match[10].str()),
                    safe_stof(match[11].str())}});
        } else if (std::regex_match(line, match, gun_key_binding_reg)) {
            key_bindings.add_gun_key_binding(GunKeyBinding{
                base: {
                    key: match[2].str(),
                    gamepad_button: match[3].str(),
                    joystick_axis: match[4].str(),
                    joystick_axis_sign: match[5].str().empty() ? 0 : safe_stof(match[5].str())},
                node: scene.get_node(match[1].str())});
        } else if (std::regex_match(line, match, console_log_reg)) {
            auto node = scene.get_node(match[1].str());
            auto mv = node->get_absolute_movable();
            auto lo = dynamic_cast<StatusWriter*>(mv);
            if (lo == nullptr) {
                throw std::runtime_error("Could not find loggable");
            }
            unsigned int log_components = safe_stoi(match[2].str());
            auto logger = std::make_shared<MovableLogger>(*node, physics_engine.advance_times_, lo, log_components);
            physics_engine.advance_times_.add_advance_time(logger);
        } else if (std::regex_match(line, match, visual_global_log_reg)) {
            auto logger = std::make_shared<VisualGlobalLog>(
                base_log,
                fpath(match[1].str()),
                FixedArray<float, 2>{
                    safe_stof(match[2].str()),
                    safe_stof(match[3].str())},
                safe_stof(match[4].str()),
                safe_stof(match[5].str()),
                safe_stoz(match[6].str()),
                log_entry_severity_from_string(match[7].str()));
            render_logics.append(nullptr, logger);
        } else if (std::regex_match(line, match, visual_node_status_reg)) {
            auto node = scene.get_node(match[1].str());
            auto mv = node->get_absolute_movable();
            auto lo = dynamic_cast<StatusWriter*>(mv);
            if (lo == nullptr) {
                throw std::runtime_error("Could not find loggable");
            }
            unsigned int log_components = safe_stoi(match[2].str());
            auto logger = std::make_shared<VisualMovableLogger>(
                *node,
                physics_engine.advance_times_,
                lo,
                log_components,
                fpath(match[3].str()),
                FixedArray<float, 2>{
                    safe_stof(match[4].str()),
                    safe_stof(match[5].str())},
                safe_stof(match[6].str()),
                safe_stof(match[7].str()));
            render_logics.append(node, logger);
            physics_engine.advance_times_.add_advance_time(logger);
        } else if (std::regex_match(line, match, visual_node_status_3rd_reg)) {
            auto node = scene.get_node(match[1].str());
            auto mv = node->get_absolute_movable();
            auto lo = dynamic_cast<StatusWriter*>(mv);
            if (lo == nullptr) {
                throw std::runtime_error("Could not find loggable");
            }
            unsigned int log_components = safe_stoi(match[2].str());
            auto logger = std::make_shared<VisualMovable3rdLogger>(
                scene_logic,
                *node,
                physics_engine.advance_times_,
                lo,
                log_components,
                fpath(match[3].str()),
                FixedArray<float, 2>{
                    safe_stof(match[4].str()),
                    safe_stof(match[5].str())},
                safe_stof(match[6].str()),
                safe_stof(match[7].str()));
            render_logics.append(node, logger);
            physics_engine.advance_times_.add_advance_time(logger);
        } else if (std::regex_match(line, match, countdown_reg)) {
            auto countdown_logic = std::make_shared<CountDownLogic>(
                fpath(match[1].str()),            // ttf_filename
                FixedArray<float, 2>{             // position
                    safe_stof(match[2].str()),
                    safe_stof(match[3].str())},
                safe_stof(match[4].str()),        // font_height_pixels
                safe_stof(match[5].str()),        // line_distance_pixels
                ui_focus.focus,
                safe_stof(match[6].str()));       // nseconds
            render_logics.append(nullptr, countdown_logic);
        } else if (std::regex_match(line, match, loading_reg)) {
            auto loading_logic = std::make_shared<LoadingTextLogic>(
                fpath(match[1].str()),            // ttf_filename
                FixedArray<float, 2>{             // position
                    safe_stof(match[2].str()),
                    safe_stof(match[3].str())},
                safe_stof(match[4].str()),        // font_height_pixels
                safe_stof(match[5].str()),        // line_distance_pixels
                ui_focus.focus,
                match[6].str());                  // text
            render_logics.append(nullptr, loading_logic);
        } else if (std::regex_match(line, match, players_stats_reg)) {
            auto players_stats_logic = std::make_shared<PlayersStatsLogic>(
                players,
                fpath(match[1].str()),            // ttf_filename
                FixedArray<float, 2>{             // position
                    safe_stof(match[2].str()),
                    safe_stof(match[3].str())},
                safe_stof(match[4].str()),        // font_height_pixels
                safe_stof(match[5].str()));       // line_distance_pixels
            render_logics.append(nullptr, players_stats_logic);
        } else if (std::regex_match(line, match, scene_selector_reg)) {
            std::list<SceneEntry> scene_entries;
            for (const auto& e : find_all_name_values(match[7].str(), "[\\w-. \\(\\)/+-:]+", "[\\w-. \\(\\)/+-:]+")) {
                scene_entries.push_back(SceneEntry{
                    name: e.first,
                    filename: fpath(e.second)});
            }
            auto scene_selector_logic = std::make_shared<SceneSelectorLogic>(
                std::vector<SceneEntry>{scene_entries.begin(), scene_entries.end()},
                fpath(match[2].str()),            // ttf_filename
                FixedArray<float, 2>{             // position
                    safe_stof(match[3].str()),
                    safe_stof(match[4].str())},
                safe_stof(match[5].str()),        // font_height_pixels
                safe_stof(match[6].str()),        // line_distance_pixels
                ui_focus,
                ui_focus.n_submenus++,
                next_scene_filename,
                num_renderings,
                button_press,
                selection_ids[match[1].str()]);
            render_logics.append(nullptr, scene_selector_logic);
        } else if (std::regex_match(line, match, clear_parameters_reg)) {
            substitutions.clear();
        } else if (std::regex_match(line, match, parameter_setter_reg)) {
            std::list<ReplacementParameter> rps;
            for (const auto& e : find_all_name_values(match[8].str(), "[\\w+-. ]+", substitute_pattern)) {
                rps.push_back(ReplacementParameter{
                    name: e.first,
                    substitutions: SubstitutionString{e.second}});
            }
            if (selection_ids.find(match[1].str()) == selection_ids.end()) {
                selection_ids.insert({match[1].str(), safe_stoi(match[7].str())});
            }
            auto parameter_setter_logic = std::make_shared<ParameterSetterLogic>(
                std::vector<ReplacementParameter>{rps.begin(), rps.end()},
                fpath(match[2].str()),            // ttf_filename
                FixedArray<float, 2>{             // position
                    safe_stof(match[3].str()),
                    safe_stof(match[4].str())},
                safe_stof(match[5].str()),        // font_height_pixels
                safe_stof(match[6].str()),        // line_distance_pixels
                ui_focus,
                ui_focus.n_submenus++,
                substitutions,
                num_renderings,
                button_press,
                selection_ids.at(match[1].str()));
            render_logics.append(nullptr, parameter_setter_logic);
        } else if (std::regex_match(line, match, ui_background_reg)) {
            auto bg = std::make_shared<MainMenuBackgroundLogic>(
                rendering_resources,
                fpath(match[1].str()),
                ui_focus.focus,
                focus_from_string(match[2].str()));
            render_logics.append(nullptr, bg);
        } else if (std::regex_match(line, match, set_renderable_style_reg)) {
            auto node = scene.get_node(match[2].str());
            node->set_style(new Style{
                .selector = std::regex{match[1].str()},
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
                    safe_stof(match[11].str())}});
        } else if (std::regex_match(line, match, hud_image_reg)) {
            auto node = scene.get_node(match[1].str());
            auto hud_image = std::make_shared<HudImageLogic>(
                *node,
                physics_engine.advance_times_,
                rendering_resources,
                fpath(match[2].str()),
                FixedArray<float, 2>{
                    safe_stof(match[3].str()),
                    safe_stof(match[4].str())},
                FixedArray<float, 2>{
                    safe_stof(match[5].str()),
                    safe_stof(match[6].str())});
            render_logics.append(node, hud_image);
            physics_engine.advance_times_.add_advance_time(hud_image);
        } else if (std::regex_match(line, match, perspective_camera_reg)) {
            auto node = scene.get_node(match[1].str());
            node->set_camera(std::make_shared<GenericCamera>(camera_config, GenericCamera::Mode::PERSPECTIVE));
            node->get_camera()->set_y_fov(safe_stof(match[2].str()));
            node->get_camera()->set_near_plane(safe_stof(match[3].str()));
            node->get_camera()->set_far_plane(safe_stof(match[4].str()));
            node->get_camera()->set_requires_postprocessing(safe_stoi(match[5].str()));
        } else if (std::regex_match(line, match, ortho_camera_reg)) {
            auto node = scene.get_node(match[1].str());
            node->set_camera(std::make_shared<GenericCamera>(camera_config, GenericCamera::Mode::ORTHO));
            node->get_camera()->set_near_plane(safe_stof(match[2].str()));
            node->get_camera()->set_far_plane(safe_stof(match[3].str()));
            node->get_camera()->set_left_plane(safe_stof(match[4].str()));
            node->get_camera()->set_right_plane(safe_stof(match[5].str()));
            node->get_camera()->set_bottom_plane(safe_stof(match[6].str()));
            node->get_camera()->set_top_plane(safe_stof(match[7].str()));
            node->get_camera()->set_requires_postprocessing(safe_stoi(match[8].str()));
        } else if (std::regex_match(line, match, light_reg)) {
            std::lock_guard lock_guard{mutex};
            std::string node_name = match[1].str();
            SceneNode* node = scene.get_node(node_name);
            render_logics.prepend(node, std::make_shared<LightmapLogic>(
                read_pixels_logic,
                rendering_resources,
                lightmap_update_cycle_from_string(match[3].str()),
                node_name,
                match[2].str(),               // black_node_name
                safe_stob(match[4].str())));  // with_depth_texture
            node->add_light(new Light{
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
                .shadow = safe_stob(match[14].str())});
        } else if (std::regex_match(line, match, look_at_node_reg)) {
            auto follower_node = scene.get_node(match[1].str());
            auto followed_node = scene.get_node(match[2].str());
            auto follower = std::make_shared<LookAtMovable>(
                physics_engine.advance_times_,
                scene,
                match[1].str(),
                followed_node,
                followed_node->get_absolute_movable());
            linker.link_absolute_movable(*follower_node, follower);
        } else if (std::regex_match(line, match, keep_offset_reg)) {
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
        } else if (std::regex_match(line, match, yaw_pitch_look_at_nodes_reg)) {
            auto yaw_node = scene.get_node(match[1].str());
            auto pitch_node = scene.get_node(match[2].str());
            auto follower_node = scene.get_node(match[3].str());
            auto follower_rb = dynamic_cast<RigidBody*>(follower_node->get_absolute_movable());
            if (follower_rb == nullptr) {
                throw std::runtime_error("Follower movable is not a rigid body");
            }
            SceneNode* followed_node = nullptr;
            RigidBody* followed_rb = nullptr;
            if (!match[4].str().empty()) {
                followed_node = scene.get_node(match[4].str());
                followed_rb = dynamic_cast<RigidBody*>(followed_node->get_absolute_movable());
                if (followed_rb == nullptr) {
                    throw std::runtime_error("Followed movable is not a rigid body");
                }
            }
            auto follower = std::make_shared<YawPitchLookAtNodes>(
                followed_node,
                physics_engine.advance_times_,
                follower_rb->rbi_,
                followed_rb == nullptr ? nullptr : &followed_rb->rbi_,
                safe_stof(match[5].str()),
                safe_stof(match[6].str()),
                safe_stof(match[7].str()),
                physics_engine_config);
            linker.link_relative_movable(*yaw_node, follower);
            linker.link_relative_movable(*pitch_node, follower->pitch_look_at_node());
        } else if (std::regex_match(line, match, follow_node_reg)) {
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
                physics_engine_config.dt);
            linker.link_absolute_movable(*follower_node, follower);
        } else if (std::regex_match(line, match, add_texture_descriptor_reg)) {
            rendering_resources.add_texture_descriptor(
                match[1].str(),
                TextureDescriptor{
                    .color = fpath(match[2].str()),
                    .normal = fpath(match[3].str()),
                    .color_mode = color_mode_from_string(match[4].str()),
                    .histogram = fpath(match[5].str()),
                    .mixed = match[6].str(),
                    .overlap_npixels = safe_stoz(match[7].str()),
                    .mean_color = OrderableFixedArray<float, 3>{
                        safe_stof(match[8].str()),
                        safe_stof(match[9].str()),
                        safe_stof(match[10].str())}});
        } else if (std::regex_match(line, match, record_track_reg)) {
            auto recorder_node = scene.get_node(match[1].str());
            auto rb = dynamic_cast<RigidBody*>(recorder_node->get_absolute_movable());
            if (rb == nullptr) {
                throw std::runtime_error("Absolute movable is not a rigid body");
            }
            physics_engine.advance_times_.add_advance_time(std::make_shared<RigidBodyRecorder>(
                fpath(match[2].str()),
                physics_engine.advance_times_,
                recorder_node,
                &rb->rbi_,
                ui_focus.focus));
        } else if (std::regex_match(line, match, playback_track_reg)) {
            auto playback_node = scene.get_node(match[1].str());
            auto playback = std::make_shared<RigidBodyPlayback>(
                fpath(match[3].str()),
                physics_engine.advance_times_,
                ui_focus.focus,
                safe_stof(match[2].str()));
            linker.link_absolute_movable(*playback_node, playback);
        } else if (std::regex_match(line, match, check_points_reg)) {
            auto moving_node = scene.get_node(match[1].str());
            physics_engine.advance_times_.add_advance_time(std::make_shared<CheckPoints>(
                fpath(match[8].str()),                  // filename
                physics_engine.advance_times_,
                moving_node,
                moving_node->get_absolute_movable(),
                match[2].str(),                         // resource
                &players,
                &players.get_player(match[3].str()),
                safe_stoi(match[4].str()),              // nbeacons
                safe_stoi(match[5].str()),              // nth
                safe_stoi(match[6].str()),              // nahead
                safe_stof(match[7].str()),              // radius
                scene_node_resources,
                scene));
        } else if (std::regex_match(line, match, set_camera_cycle_reg)) {
            std::string cameras = match[2].str();
            auto& cycle = (match[1].str() == "near")
                ? selected_cameras.camera_cycle_near
                : selected_cameras.camera_cycle_far;
            cycle = string_to_vector(cameras);
        } else if (std::regex_match(line, match, set_camera_reg)) {
            selected_cameras.set_camera_node_name(match[1].str());
        } else if (std::regex_match(line, match, set_dirtmap_reg)) {
            dirtmap_logic.set_filename(fpath(match[1].str()));
            rendering_resources.set_discreteness("dirtmap", safe_stof(match[2].str()));
            rendering_resources.set_texture_wrap("dirtmap", clamp_mode_from_string(match[3].str()));
        } else if (std::regex_match(line, match, set_skybox_reg)) {
            skybox_logic.set_filenames({
                fpath(match[2].str()),
                fpath(match[3].str()),
                fpath(match[4].str()),
                fpath(match[5].str()),
                fpath(match[6].str()),
                fpath(match[7].str())},
                match[1].str());
        } else if (std::regex_match(line, match, set_preferred_car_spawner_reg)) {
            std::string player = match[1].str();
            std::string macro = match[2].str();
            std::string parameters = match[3].str();
            game_logic.set_preferred_car_spawner(
                players.get_player(player),
                [macro_line_executor, player, macro, parameters, &rsc](const SpawnPoint& p){
                    std::stringstream sstr;
                    sstr <<
                        "macro_playback " <<
                        macro <<
                        " CAR_NODE_X:" << p.position(0) <<
                        " CAR_NODE_Y:" << p.position(1) <<
                        " CAR_NODE_Z:" << p.position(2) <<
                        " CAR_NODE_ANGLE_X:" << 180.f / M_PI * p.rotation(0) <<
                        " CAR_NODE_ANGLE_Y:" << 180.f / M_PI * p.rotation(1) <<
                        " CAR_NODE_ANGLE_Z:" << 180.f / M_PI * p.rotation(2) <<
                        " " << parameters <<
                        " -SUFFIX:_" << player <<
                        " IF_WITH_PHYSICS:" << 
                        " IF_RACING:#" << 
                        " IF_RALLY:" <<
                        " PLAYER_NAME:" << player;
                    macro_line_executor(sstr.str(), rsc);
                }
            );
        } else if (std::regex_match(line, match, set_vip_reg)) {
            game_logic.set_vip(&players.get_player(match[1].str()));
        } else if (std::regex_match(line, match, set_spawn_points_reg)) {
            SceneNode* node = scene.get_node(match[1].str());
            std::list<SpawnPoint> spawn_points = scene_node_resources.spawn_points(match[2].str());
            game_logic.set_spawn_points(*node, spawn_points);
        } else if (std::regex_match(line, match, set_way_points_reg)) {
            Player& player = players.get_player(match[1].str());
            SceneNode* node = scene.get_node(match[2].str());
            PointsAndAdjacency<float, 2> way_points = scene_node_resources.way_points(match[3].str());
            player.set_waypoints(*node, way_points);
        } else if (std::regex_match(line, match, burn_in_reg)) {
            physics_engine.burn_in(safe_stof(match[1].str()));
        } else if (std::regex_match(line, match, append_focus_reg)) {
            ui_focus.focus.push_back(focus_from_string(match[1].str()));
        } else {
            return false;
        }
        return true;
    };
    MacroLineExecutor lp2{
        macro_file_executor_,
        script_filename,
        fs::path(scene_filename).parent_path().string(),
        execute_user_function,
        substitutions,
        verbose};
    macro_file_executor_(lp2, rsc);
}
