#include "Load_Scene.hpp"
#include <Mlib/Math/Pi.hpp>
#include <Mlib/Math/Rodrigues.hpp>
#include <Mlib/Physics/Containers/Players.hpp>
#include <Mlib/Physics/Objects/Check_Points.hpp>
#include <Mlib/Physics/Objects/Damageable.hpp>
#include <Mlib/Physics/Objects/Follow_Movable.hpp>
#include <Mlib/Physics/Objects/Gun.hpp>
#include <Mlib/Physics/Objects/Keep_Offset_Movable.hpp>
#include <Mlib/Physics/Objects/Look_At_Movable.hpp>
#include <Mlib/Physics/Objects/Movable_Logger.hpp>
#include <Mlib/Physics/Objects/Pitch_Look_At_Node.hpp>
#include <Mlib/Physics/Objects/Player.hpp>
#include <Mlib/Physics/Objects/Relative_Transformer.hpp>
#include <Mlib/Physics/Objects/Rigid_Body.hpp>
#include <Mlib/Physics/Objects/Rigid_Body_Engine.hpp>
#include <Mlib/Physics/Objects/Rigid_Body_Playback.hpp>
#include <Mlib/Physics/Objects/Rigid_Body_Recorder.hpp>
#include <Mlib/Physics/Objects/Rigid_Primitives.hpp>
#include <Mlib/Physics/Objects/Trigger_Gun_Ai.hpp>
#include <Mlib/Physics/Objects/Wheel.hpp>
#include <Mlib/Physics/Objects/Yaw_Pitch_Look_At_Nodes.hpp>
#include <Mlib/Physics/Physics_Engine.hpp>
#include <Mlib/Regex.hpp>
#include <Mlib/Render/Cameras/Generic_Camera.hpp>
#include <Mlib/Render/Key_Bindings/Absolute_Movable_Idle_Binding.hpp>
#include <Mlib/Render/Key_Bindings/Absolute_Movable_Key_Binding.hpp>
#include <Mlib/Render/Key_Bindings/Camera_Key_Binding.hpp>
#include <Mlib/Render/Key_Bindings/Gun_Key_Binding.hpp>
#include <Mlib/Render/Key_Bindings/Relative_Movable_Key_Binding.hpp>
#include <Mlib/Render/Render_Logics/Countdown_Logic.hpp>
#include <Mlib/Render/Render_Logics/Dirtmap_Logic.hpp>
#include <Mlib/Render/Render_Logics/Lightmap_Logic.hpp>
#include <Mlib/Render/Render_Logics/Loading_Text_Logic.hpp>
#include <Mlib/Render/Render_Logics/Main_Menu_Background_Logic.hpp>
#include <Mlib/Render/Render_Logics/Read_Pixels_Logic.hpp>
#include <Mlib/Render/Render_Logics/Render_Logics.hpp>
#include <Mlib/Render/Render_Logics/Skybox_Logic.hpp>
#include <Mlib/Render/Renderables/Renderable_Binary_X.hpp>
#include <Mlib/Render/Renderables/Renderable_Blending_Square.hpp>
#include <Mlib/Render/Renderables/Renderable_Blending_X.hpp>
#include <Mlib/Render/Renderables/Renderable_Obj_File.hpp>
#include <Mlib/Render/Renderables/Renderable_Osm_Map.hpp>
#include <Mlib/Render/Rendering_Resources.hpp>
#include <Mlib/Render/Selected_Cameras.hpp>
#include <Mlib/Render/Ui/Button_Press.hpp>
#include <Mlib/Scene/Render_Logics/Hud_Image_Logic.hpp>
#include <Mlib/Scene/Render_Logics/Parameter_Setter_Logic.hpp>
#include <Mlib/Scene/Render_Logics/Players_Stats_Logic.hpp>
#include <Mlib/Scene/Render_Logics/Scene_Selector_Logic.hpp>
#include <Mlib/Scene/Render_Logics/Visual_Movable_3rd_Logger.hpp>
#include <Mlib/Scene/Render_Logics/Visual_Movable_Logger.hpp>
#include <Mlib/Scene_Graph/Scene.hpp>
#include <Mlib/Scene_Graph/Scene_Node_Resources.hpp>
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
    void link_absolute_movable(SceneNode& node, const std::shared_ptr<TAbsoluteMovable>& absolute_movable) {
        // 1. Set movable, which updates the transformation-matrix
        node.set_absolute_movable(absolute_movable.get());
        // 2. Add to physics engine
        advance_times_.add_advance_time(absolute_movable);
    };
    template <class TRelativeMovable>
    void link_relative_movable(SceneNode& node, const std::shared_ptr<TRelativeMovable>& relative_movable) {
        node.set_relative_movable(relative_movable.get());
        advance_times_.add_advance_time(relative_movable);
    };
    template <class TAbsoluteObserver>
    void link_absolute_observer(SceneNode& node, const std::shared_ptr<TAbsoluteObserver>& absolute_observer) {
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
    std::vector<CameraKeyBinding>& camera_key_bindings,
    std::vector<AbsoluteMovableIdleBinding>& absolute_movable_idle_bindings,
    std::vector<AbsoluteMovableKeyBinding>& absolute_movable_key_bindings,
    std::vector<RelativeMovableKeyBinding>& relative_movable_key_bindings,
    std::vector<GunKeyBinding>& gun_key_bindings,
    SelectedCameras& selected_cameras,
    const CameraConfig& camera_config,
    const PhysicsEngineConfig& physics_engine_config,
    RenderLogics& render_logics,
    RenderLogic& scene_logic,
    ReadPixelsLogic& read_pixels_logic,
    DirtmapLogic& dirtmap_logic,
    SkyboxLogic& skybox_logic,
    UiFocus& ui_focus,
    SubstitutionString& substitutions,
    size_t& num_renderings,
    std::map<std::string, size_t>& selection_ids,
    bool verbose)
{
    std::ifstream ifs{script_filename};
    const std::regex osm_resource_reg(
        "^(?:\\r?\\n|\\s)*osm_resource\\r?\\n"
        "\\s*name=([\\w+-.]+)\\r?\\n"
        "\\s*filename=([\\w-. \\(\\)/+-]+)\\r?\\n"
        "\\s*heightmap=([\\w-. \\(\\)/+-]*)\\r?\\n"
        "\\s*terrain_texture=([\\w-. \\(\\)/+-]*)\\r?\\n"
        "\\s*dirt_texture=([\\w-. \\(\\)/+-]*)\\r?\\n"
        "\\s*asphalt_texture=([\\w-. \\(\\)/+-]*)\\r?\\n"
        "\\s*street_texture=([\\w-. \\(\\)/+-]*)\\r?\\n"
        "\\s*path_texture=([\\w-. \\(\\)/+-]*)\\r?\\n"
        "\\s*curb_street_texture=([\\w-. \\(\\)/+-]*)\\r?\\n"
        "\\s*curb_path_texture=([\\w-. \\(\\)/+-]*)\\r?\\n"
        "\\s*facade_texture=([\\w-. \\(\\)/+-]*)\\r?\\n"
        "\\s*facade_texture_2=([\\w-. \\(\\)/+-]*)\\r?\\n"
        "\\s*facade_texture_3=([\\w-. \\(\\)/+-]*)\\r?\\n"
        "\\s*ceiling_texture=([\\w-. \\(\\)/+-]*)\\r?\\n"
        "\\s*barrier_texture=([\\w-. \\(\\)/+-]*)\\r?\\n"
        "\\s*barrier_blend_mode=(off|binary|continuous)\\r?\\n"
        "\\s*roof_texture=([\\w-. \\(\\)/+-]*)\\r?\\n"
        "\\s*tree_resource_names=([\\s\\w-. \\(\\)/+-]*)\\r?\\n"
        "\\s*grass_resource_names=([\\s\\w-. \\(\\)/+-]*)\\r?\\n"
        "\\s*default_street_width=([\\w+-.]+)\\r?\\n"
        "\\s*roof_width=([\\w+-.]+)\\r?\\n"
        "\\s*scale=([\\w+-.]+)\\r?\\n"
        "\\s*uv_scale=([\\w+-.]+)\\r?\\n"
        "\\s*uv_scale_facade=([\\w+-.]+)\\r?\\n"
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
        "\\s*with_terrain=(0|1)\\r?\\n"
        "\\s*with_buildings=(0|1)\\r?\\n"
        "\\s*only_raceways=(0|1)\\r?\\n"
        "\\s*steiner_point_distance=([\\w+-.]+)\\r?\\n"
        "\\s*curb_alpha=([\\w+-.]+)\\r?\\n"
        "\\s*raise_streets_amount=([\\w+-.]+)\\r?\\n"
        "\\s*add_street_lights=(0|1)\\r?\\n"
        "\\s*max_wall_width=([\\w+-.]+)\\r?\\n"
        "\\s*with_height_bindings=(0|1)$");
    const std::regex obj_resource_reg(
        "^(?:\\r?\\n|\\s)*obj_resource\\r?\\n"
        "\\s*name=([\\w-. \\(\\)/+-]+)\\r?\\n"
        "\\s*filename=([\\w-. \\(\\)/+-]+)\\r?\\n"
        "\\s*position=([\\w+-.]+) ([\\w+-.]+) ([\\w+-.]+)\\r?\\n"
        "\\s*rotation=([\\w+-.]+) ([\\w+-.]+) ([\\w+-.]+)\\r?\\n"
        "\\s*scale=([\\w+-.]+) ([\\w+-.]+) ([\\w+-.]+)\\r?\\n"
        "\\s*is_small=(0|1)\\r?\\n"
        "\\s*blend_mode=(off|binary|continuous)\\r?\\n"
        "\\s*blend_cull_faces=(0|1)\\r?\\n"
        "\\s*occluded_type=(off|color|depth)\\r?\\n"
        "\\s*occluder_type=(off|white|black)\\r?\\n"
        "\\s*occluded_by_black=(0|1)\\r?\\n"
        "\\s*aggregate_mode=(off|once|sorted)(\\r?\\n"
        "\\s*no_werror)?$");
    const std::regex gen_triangle_rays_reg("^(?:\\r?\\n|\\s)*gen_triangle_rays name=([\\w+-.]+) npoints=([\\w+-.]+) lengths=([\\w+-.]+) ([\\w+-.]+) ([\\w+-.]+) delete_triangles=(0|1)$");
    const std::regex gen_ray_reg("^(?:\\r?\\n|\\s)*gen_ray name=([\\w+-.]+) from=([\\w+-.]+) ([\\w+-.]+) ([\\w+-.]+) to=([\\w+-.]+) ([\\w+-.]+) ([\\w+-.]+)$");
    const std::regex blending_square_resource_reg("^(?:\\r?\\n|\\s)*blending_square_resource name=([\\w+-.]+) texture_filename=([\\w-. \\(\\)/+-]+) min=([\\w+-.]+) ([\\w+-.]+) max=([\\w+-.]+) ([\\w+-.]+)$");
    const std::regex blending_x_resource_reg("^(?:\\r?\\n|\\s)*blending_x_resource name=([\\w+-.]+) texture_filename=([\\w-. \\(\\)/+-]+) min=([\\w+-.]+) ([\\w+-.]+) max=([\\w+-.]+) ([\\w+-.]+)$");
    const std::regex binary_x_resource_reg("^(?:\\r?\\n|\\s)*binary_x_resource name=([\\w+-.]+) texture_filename=([\\w-. \\(\\)/+-]+) min=([\\w+-.]+) ([\\w+-.]+) max=([\\w+-.]+) ([\\w+-.]+) ambience=([\\w+-.]+) ([\\w+-.]+) ([\\w+-.]+) is_small=(0|1)$");
    const std::regex node_instance_reg("^(?:\\r?\\n|\\s)*node_instance parent=([\\w-.<>]+) name=([\\w+-.]+) position=([\\w+-.]+) ([\\w+-.]+) ([\\w+-.]+) rotation=([\\w+-.]+) ([\\w+-.]+) ([\\w+-.]+) scale=([\\w+-.]+)(?: aggregate=(true|false))?$");
    const std::regex renderable_instance_reg("^(?:\\r?\\n|\\s)*renderable_instance name=([\\w+-.]+) node=([\\w+-.]+) resource=([\\w-. \\(\\)/+-]+)(?: regex=(.*))?$");
    const std::regex rigid_cuboid_reg("^(?:\\r?\\n|\\s)*rigid_cuboid node=([\\w+-.]+) hitbox=([\\w-. \\(\\)/+-]+)(?: tirelines=([\\w-. \\(\\)/+-]+))? mass=([\\w+-.]+) size=([\\w+-.]+) ([\\w+-.]+) ([\\w+-.]+)$");
    const std::regex gun_reg("^(?:\\r?\\n|\\s)*gun node=([\\w+-.]+) parent_rigid_body_node=([\\w+-.]+) cool-down=([\\w+-.]+) renderable=([\\w-. \\(\\)/+-]+) hitbox=([\\w-. \\(\\)/+-]+) mass=([\\w+-.]+) velocity=([\\w+-.]+) lifetime=([\\w+-.]+) damage=([\\w+-.]+) size=([\\w+-.]+) ([\\w+-.]+) ([\\w+-.]+)$");
    const std::regex trigger_gun_ai_reg("^(?:\\r?\\n|\\s)*trigger_gun_ai base_shooter_node=([\\w+-.]+) base_target_node=([\\w+-.]+) gun_node=([\\w+-.]+)$");
    const std::regex damageable_reg("^(?:\\r?\\n|\\s)*damageable node=([\\w+-.]+) health=([\\w+-.]+)$");
    const std::regex relative_transformer_reg("^(?:\\r?\\n|\\s)*relative_transformer node=([\\w+-.]+)$");
    const std::regex wheel_reg("^(?:\\r?\\n|\\s)*wheel rigid_body=([\\w+-.]+) node=([\\w+-.]+) radius=([\\w+-.]+) tire_id=(\\d+)$");
    const std::regex create_engine_reg("^(?:\\r?\\n|\\s)*create_engine rigid_body=([\\w+-.]+) name=([\\w+-.]+) power=([\\w+-.]+) tires=([\\d\\s]*)$");
    const std::regex player_create_reg("^(?:\\r?\\n|\\s)*player_create name=([\\w+-.]+) team=([\\w+-.]+)$");
    const std::regex player_set_node_reg("^(?:\\r?\\n|\\s)*player_set_node player-name=([\\w+-.]+) node=([\\w+-.]+)$");
    const std::regex player_set_aiming_gun_reg("^(?:\\r?\\n|\\s)*player_set_aiming_gun player-name=([\\w+-.]+) yaw_node=([\\w+-.]+) gun_node=([\\w+-.]*)$");
    const std::regex player_set_surface_power_reg("^(?:\\r?\\n|\\s)*player_set_surface_power player-name=([\\w+-.]+) forward=([\\w+-.]+) backward=([\\w+-.]*)$");
    const std::regex player_set_tire_angle_reg("^(?:\\r?\\n|\\s)*player_set_tire_angle player-name=([\\w+-.]+) tire_id=(\\d+) tire_angle_left=([\\w+-.]*) tire_angle_right=([\\w+-.]*)$");
    const std::regex player_set_waypoint_reg("^(?:\\r?\\n|\\s)*player_set_waypoint player-name=([\\w+-.]+) position=([\\w+-.]*) ([\\w+-.]*)$");
    const std::regex team_set_waypoint_reg("^(?:\\r?\\n|\\s)*team_set_waypoint team-name=([\\w+-.]+) position=([\\w+-.]*) ([\\w+-.]*)$");
    const std::regex camera_key_binding_reg("^(?:\\r?\\n|\\s)*camera_key_binding key=([\\w+-.]+) gamepad_button=([\\w+-.]*) joystick_digital_axis=([\\w+-.]*) joystick_digital_axis_sign=([\\w+-.]+)$");
    const std::regex abs_idle_binding_reg(
        "^(?:\\r?\\n|\\s)*abs_idle_binding\\r?\\n"
        "\\s*node=([\\w+-.]+)\\r?\\n"
        "\\s*tires_z=([\\w+-.]+) ([\\w+-.]+) ([\\w+-.]+)$");
    const std::regex abs_key_binding_reg(
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
        "\\s*tire_angle=([\\w+-.]+))?"
        "(?:\\r?\\n\\s*tires_z=([\\w+-.]+) ([\\w+-.]+) ([\\w+-.]+))?$");
    const std::regex rel_key_binding_reg(
        "^(?:\\r?\\n|\\s)*rel_key_binding\\r?\\n"
        "\\s*node=([\\w+-.]+)\\r?\\n"
        "\\s*key=([\\w+-.]+)"
        "(?:\\r?\\n\\s*gamepad_button=([\\w+-.]*))?"
        "\\s*joystick_digital_axis=([\\w+-.]*)\\r?\\n"
        "\\s*joystick_digital_axis_sign=([\\w+-.]+)\\r?\\n"
        "\\s*angular_velocity_press=([\\w+-.]+) ([\\w+-.]+) ([\\w+-.]+)\\r?\\n"
        "\\s*angular_velocity_repeat=([\\w+-.]+) ([\\w+-.]+) ([\\w+-.]+)$");
    const std::regex gun_key_binding_reg(
        "^(?:\\r?\\n|\\s)*gun_key_binding\\r?\\n"
        "\\s*node=([\\w+-.]+)\\r?\\n"
        "\\s*key=([\\w+-.]+)"
        "(?:\\r?\\n\\s*gamepad_button=([\\w+-.]*))?"
        "(?:\\r?\\n\\s*joystick_digital_axis=([\\w+-.]+)\\r?\\n"
        "\\s*joystick_digital_axis_sign=([\\w+-.]+))?$");
    const std::regex console_log_reg("^(?:\\r?\\n|\\s)*console_log node=([\\w+-.]+) format=(\\d+)$");
    const std::regex visual_log_reg("^(?:\\r?\\n|\\s)*visual_log node=([\\w+-.]+) format=(\\d+) ttf_file=([\\w-. \\(\\)/+-]+) position=([\\w+-.]+) ([\\w+-.]+) font_height=([\\w+-.]+) line_distance=([\\w+-.]+)$");
    const std::regex visual_log_3rd_reg("^(?:\\r?\\n|\\s)*visual_log_3rd node=([\\w+-.]+) format=(\\d+) ttf_file=([\\w-. \\(\\)/+-]+) offset=([\\w+-.]+) ([\\w+-.]+) font_height=([\\w+-.]+) line_distance=([\\w+-.]+)$");
    const std::regex loading_reg(
        "^(?:\\r?\\n|\\s)*loading"
        "\\s*ttf_file=([\\w-. \\(\\)/+-]+)\\r?\\n"
        "\\s*position=([\\w+-.]+) ([\\w+-.]+)\\r?\\n"
        "\\s*font_height=([\\w+-.]+)\\r?\\n"
        "\\s*line_distance=([\\w+-.]+)\\r?\\n"
        "\\s*text=(.*)$");
    const std::regex countdown_reg(
        "^(?:\\r?\\n|\\s)*countdown"
        "\\s*ttf_file=([\\w-. \\(\\)/+-]+)\\r?\\n"
        "\\s*position=([\\w+-.]+) ([\\w+-.]+)\\r?\\n"
        "\\s*font_height=([\\w+-.]+)\\r?\\n"
        "\\s*line_distance=([\\w+-.]+)\\r?\\n"
        "\\s*nseconds=([\\w+-.]+)$");
    const std::regex players_stats_reg(
        "^(?:\\r?\\n|\\s)*players_stats\\r?\\n"
        "\\s*ttf_file=([\\w-. \\(\\)/+-]+)\\r?\\n"
        "\\s*position=([\\w+-.]+) ([\\w+-.]+)\\r?\\n"
        "\\s*font_height=([\\w+-.]+)\\r?\\n"
        "\\s*line_distance=([\\w+-.]+)$");
    const std::regex scene_selector_reg(
        "^(?:\\r?\\n|\\s)*scene_selector\\r?\\n"
        "\\s*id=([\\w+-.]+)\\r?\\n"
        "\\s*ttf_file=([\\w-. \\(\\)/+-]+)\\r?\\n"
        "\\s*position=([\\w+-.]+) ([\\w+-.]+)\\r?\\n"
        "\\s*font_height=([\\w+-.]+)\\r?\\n"
        "\\s*line_distance=([\\w+-.]+)\\r?\\n"
        "\\s*scene_files=([\\r\\n\\w-. \\(\\)/+-:=]+)$");
    const std::regex clear_parameters_reg(
        "^(?:\\r?\\n|\\s)*clear_parameters$");
    const std::regex parameter_setter_reg(
        "^(?:\\r?\\n|\\s)*parameter_setter\\r?\\n"
        "\\s*id=([\\w+-.]+)\\r?\\n"
        "\\s*ttf_file=([\\w-. \\(\\)/+-]+)\\r?\\n"
        "\\s*position=([\\w+-.]+) ([\\w+-.]+)\\r?\\n"
        "\\s*font_height=([\\w+-.]+)\\r?\\n"
        "\\s*line_distance=([\\w+-.]+)\\r?\\n"
        "\\s*parameters=([\\r\\n\\w-. \\(\\)/+-:=]+)$");
    const std::regex ui_background_reg("^(?:\\r?\\n|\\s)*ui_background texture=([\\w-. \\(\\)/+-]+) target_focus=(menu|loading|countdown|scene)$");
    const std::regex hud_image_reg("^(?:\\r?\\n|\\s)*hud_image node=([\\w+-.]+) filename=([\\w-. \\(\\)/+-]+) center=([\\w+-.]+) ([\\w+-.]+) size=([\\w+-.]+) ([\\w+-.]+)$");
    const std::regex perspective_camera_reg("^(?:\\r?\\n|\\s)*perspective_camera node=([\\w+-.]+) y-fov=([\\w+-.]+) near_plane=([\\w+-.]+) far_plane=([\\w+-.]+) requires_postprocessing=(0|1)$");
    const std::regex ortho_camera_reg("^(?:\\r?\\n|\\s)*ortho_camera node=([\\w+-.]+) near_plane=([\\w+-.]+) far_plane=([\\w+-.]+) left_plane=([\\w+-.]+) right_plane=([\\w+-.]+) bottom_plane=([\\w+-.]+) top_plane=([\\w+-.]+) requires_postprocessing=(0|1)$");
    const std::regex light_reg("^(?:\\r?\\n|\\s)*light node=([\\w+-.]+) black_node=([\\w+-.]*) update=(once|always) with_depth_texture=(0|1) ambience=([\\w+-.]+) ([\\w+-.]+) ([\\w+-.]+) diffusivity=([\\w+-.]+) ([\\w+-.]+) ([\\w+-.]+) specularity=([\\w+-.]+) ([\\w+-.]+) ([\\w+-.]+)$");
    const std::regex look_at_node_reg("^(?:\\r?\\n|\\s)*look_at_node follower=([\\w+-.]+) followed=([\\w+-.]+)$");
    const std::regex keep_offset_reg("^(?:\\r?\\n|\\s)*keep-offset follower=([\\w+-.]+) followed=([\\w+-.]+) offset=([\\w+-.]+) ([\\w+-.]+) ([\\w+-.]+)$");
    const std::regex yaw_pitch_look_at_nodes_reg("^(?:\\r?\\n|\\s)*yaw_pitch_look_at_nodes yaw_node=([\\w+-.]+) pitch_node=([\\w+-.]+) parent_follower_rigid_body_node=([\\w+-.]+) followed=([\\w+-.]*) bullet_start_offset=([\\w+-.]+) bullet_velocity=([\\w+-.]+) gravity=([\\w+-.]+)$");
    const std::regex follow_node_reg(
        "^(?:\\r?\\n|\\s)*follow_node\\r?\\n"
        "\\s*follower=([\\w+-.]+)\\r?\\n"
        "\\s*followed=([\\w+-.]+)\\r?\\n"
        "\\s*distance=([\\w+-.]+)\\r?\\n"
        "\\s*node_displacement=([\\w+-.]+) ([\\w+-.]+) ([\\w+-.]+)\\r?\\n"
        "\\s*look_at_displacement=([\\w+-.]+) ([\\w+-.]+) ([\\w+-.]+)\\r?\\n"
        "\\s*snappiness=([\\w+-.]+)\\r?\\n"
        "\\s*y_adaptivity=([\\w+-.]+)\\r?\\n"
        "\\s*y_snappiness=([\\w+-.]+)$");
    const std::regex record_track_reg("^(?:\\r?\\n|\\s)*record_track node=([\\w+-.]+) filename=([\\w-. \\(\\)/+-]+)$");
    const std::regex playback_track_reg("^(?:\\r?\\n|\\s)*playback_track node=([\\w+-.]+) speed=([\\w+-.]+) filename=([\\w-. \\(\\)/+-]+)$");
    const std::regex check_points_reg("^(?:\\r?\\n|\\s)*check_points moving-node=([\\w+-.]+) beacon_node0=([\\w+-.]+) beacon_node1=([\\w+-.]+) player=([\\w+-.]+) nth=(\\d+) radius=([\\w+-.]+) track_filename=([\\w-. \\(\\)/+-]+)$");
    const std::regex set_camera_cycle_reg("^(?:\\r?\\n|\\s)*set_camera_cycle name=(near|far)((?: [\\w+-.]+)+)$");
    const std::regex set_camera_reg("^(?:\\r?\\n|\\s)*set_camera ([\\w+-.]+)$");
    const std::regex set_dirtmap_reg("^(?:\\r?\\n|\\s)*set_dirtmap filename=([\\w-. \\(\\)/+-]+)$");
    const std::regex set_skybox_reg("^(?:\\r?\\n|\\s)*set_skybox alias=([\\w+-.]+) filenames=([\\w-. \\(\\)/+-]+) ([\\w-. \\(\\)/+-]+) ([\\w-. \\(\\)/+-]+) ([\\w-. \\(\\)/+-]+) ([\\w-. \\(\\)/+-]+) ([\\w-. \\(\\)/+-]+)$");
    const std::regex burn_in_reg("^(?:\\r?\\n|\\s)*burn_in seconds=([\\w+-.]+)$");
    const std::regex append_focus_reg("^(?:\\r?\\n|\\s)*append_focus (menu|loading|countdown|scene)$");
    const std::regex comment_reg("^(?:\\r?\\n|\\s)*#[\\S\\s]*$");
    const std::regex macro_begin_reg("^(?:\\r?\\n|\\s)*macro_begin ([\\w+-.]+)$");
    const std::regex macro_end_reg("^(?:\\r?\\n|\\s)*macro_end$");
    const std::regex macro_playback_reg("^(?:\\r?\\n|\\s)*macro_playback(?:\\r?\\n|\\s)+([\\w+-.]+)(" + substitute_pattern + ")$");
    const std::regex include_reg("^(?:\\r?\\n|\\s)*include ([\\w-. \\(\\)/+-]+)$");
    const std::regex empty_reg("^[\\s]*$");

    auto fpath = [&](const fs::path& f) -> std::string {
        if (f.empty()) {
            return "";
        } else if (f.is_absolute()) {
            return f.string();
        } else {
            return fs::weakly_canonical(fs::path(scene_filename).parent_path() / f).string();
        }
    };

    Linker linker{physics_engine.advance_times_};

    std::function<void(const std::string&, const std::string&)> process_line = [&](
        const std::string& line,
        const std::string& line_script_filename)
    {
        auto spath = [&](const fs::path& f) -> std::string {
            if (f.empty()) {
                return "";
            } else if (f.is_absolute()) {
                return f.string();
            } else {
                return fs::canonical(fs::path(line_script_filename).parent_path() / f).string();
            }
        };

        if (verbose) {
            std::cerr << "Processing line \"" << line << '"' << std::endl;
        }
        std::smatch match;
        if (std::regex_match(line, match, empty_reg)) {
            // Do nothing
        } else if (std::regex_match(line, match, comment_reg)) {
            // Do nothing
        } else if (std::regex_match(line, match, osm_resource_reg)) {
            scene_node_resources.add_resource(
                match[1].str(),
                std::make_shared<RenderableOsmMap>(
                    scene_node_resources,
                    &rendering_resources,
                    fpath(match[2].str()),
                    fpath(match[3].str()),
                    fpath(match[4].str()),
                    fpath(match[5].str()),
                    fpath(match[6].str()),
                    fpath(match[7].str()),
                    fpath(match[8].str()),
                    fpath(match[9].str()),
                    fpath(match[10].str()),
                    fpath(match[11].str()),
                    fpath(match[12].str()),
                    fpath(match[13].str()),
                    fpath(match[14].str()),
                    fpath(match[15].str()),
                    blend_mode_from_string(match[16].str()),
                    fpath(match[17].str()),
                    string_to_vector(match[18].str()),
                    string_to_vector(match[19].str()),
                    safe_stof(match[20].str()),
                    safe_stof(match[21].str()),
                    safe_stof(match[22].str()),
                    safe_stof(match[23].str()),
                    safe_stof(match[24].str()),
                    safe_stof(match[25].str()),
                    safe_stoi(match[26].str()),
                    safe_stoi(match[27].str()),
                    safe_stof(match[28].str()),
                    safe_stof(match[29].str()),
                    safe_stof(match[30].str()),
                    safe_stoi(match[31].str()),
                    safe_stoi(match[32].str()),
                    safe_stof(match[33].str()),
                    safe_stof(match[34].str()),
                    safe_stof(match[35].str()),
                    safe_stoi(match[36].str()),
                    safe_stoi(match[37].str()),
                    safe_stoi(match[38].str()),
                    safe_stof(match[39].str()),
                    safe_stof(match[40].str()),
                    safe_stof(match[41].str()),
                    safe_stoi(match[42].str()),
                    safe_stof(match[43].str()),
                    safe_stoi(match[44].str())));
        } else if (std::regex_match(line, match, obj_resource_reg)) {
            scene_node_resources.add_resource(match[1].str(), std::make_shared<RenderableObjFile>(
                fpath(match[2].str()),
                FixedArray<float, 3>{
                    safe_stof(match[3].str()),
                    safe_stof(match[4].str()),
                    safe_stof(match[5].str())},
                FixedArray<float, 3>{
                    safe_stof(match[6].str()) / 180 * float(M_PI),
                    safe_stof(match[7].str()) / 180 * float(M_PI),
                    safe_stof(match[8].str()) / 180 * float(M_PI)},
                FixedArray<float, 3>{
                    safe_stof(match[9].str()),
                    safe_stof(match[10].str()),
                    safe_stof(match[11].str())},
                    &rendering_resources,
                safe_stoi(match[12].str()),
                blend_mode_from_string(match[13].str()),
                safe_stoi(match[14].str()),                     // blend_cull_faces
                occluded_type_from_string(match[15].str()),
                occluder_type_from_string(match[16].str()),
                safe_stoi(match[17].str()),                     // occluded_by_black
                aggregate_mode_from_string(match[18].str()),
                false,                                          // apply_static_lighting
                match[19].str() == ""));
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
        } else if (std::regex_match(line, match, blending_square_resource_reg)) {
            scene_node_resources.add_resource(match[1].str(), std::make_shared<RenderableBlendingSquare>(
                FixedArray<float, 2, 2>{
                    safe_stof(match[3].str()), safe_stof(match[4].str()),
                    safe_stof(match[5].str()), safe_stof(match[6].str())},
                fpath(match[2].str()),
                &rendering_resources));
        } else if (std::regex_match(line, match, blending_x_resource_reg)) {
            scene_node_resources.add_resource(match[1].str(), std::make_shared<RenderableBlendingX>(
                FixedArray<float, 2, 2>{
                    safe_stof(match[3].str()), safe_stof(match[4].str()),
                    safe_stof(match[5].str()), safe_stof(match[6].str())},
                fpath(match[2].str()),
                &rendering_resources));
        } else if (std::regex_match(line, match, binary_x_resource_reg)) {
            scene_node_resources.add_resource(match[1].str(), std::make_shared<RenderableBinaryX>(
                FixedArray<float, 2, 2>{
                    safe_stof(match[3].str()), safe_stof(match[4].str()),
                    safe_stof(match[5].str()), safe_stof(match[6].str())},
                fpath(match[2].str()),
                &rendering_resources,
                safe_stob(match[10].str()),
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
                    safe_stof(match[7].str())});
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
            auto d = std::make_shared<Damageable>(
                scene,
                physics_engine.advance_times_,
                match[1].str(),
                safe_stof(match[2].str()));
            rb->collision_observers_.push_back(d);
            physics_engine.advance_times_.add_advance_time(d);
        } else if (std::regex_match(line, match, relative_transformer_reg)) {
            std::shared_ptr<RelativeTransformer> rt = std::make_shared<RelativeTransformer>(physics_engine.advance_times_);
            linker.link_relative_movable(*scene.get_node(match[1].str()), rt);
        } else if (std::regex_match(line, match, wheel_reg)) {
            auto rb = dynamic_cast<RigidBody*>(scene.get_node(match[1].str())->get_absolute_movable());
            if (rb == nullptr) {
                throw std::runtime_error("Absolute movable is not a rigid body");
            }
            std::shared_ptr<Wheel> wheel = std::make_shared<Wheel>(
                *rb,
                physics_engine.advance_times_,
                (size_t)safe_stoi(match[4].str()),
                safe_stof(match[3].str()));
            linker.link_relative_movable(*scene.get_node(match[2].str()), wheel);
        } else if (std::regex_match(line, match, create_engine_reg)) {
            auto rb = dynamic_cast<RigidBody*>(scene.get_node(match[1].str())->get_absolute_movable());
            if (rb == nullptr) {
                throw std::runtime_error("Absolute movable is not a rigid body");
            }
            auto ep = rb->engines_.insert(
                std::make_pair(match[2].str(),
                RigidBodyEngine{safe_stof(match[3].str())}));
            if (!ep.second) {
                throw std::runtime_error("Engine with name \"" + match[2].str() + "\" already exists");
            }
            for(const std::string& t : string_to_list(match[4].str())) {
                ep.first->second.increment_ntires();
                // From: https://www.nanolounge.de/21977/federkonstante-und-masse-bei-auto
                // Ds = 1000 / 4 * 9.8 / 0.02 = 122500 = 1.225e5

                // Da * 1 = 1000 / 4 * 9.8 => Da = 1e4 / 4
                auto tp = rb->tires_.insert(std::make_pair(safe_stoi(t), Tire{match[2].str(), 1e5, 2e3, 0}));
                if (!tp.second) {
                    throw std::runtime_error("Tire with ID \"" + t + "\" already exists");
                }
            }
        } else if (std::regex_match(line, match, player_create_reg)) {
            auto player = std::make_shared<Player>(physics_engine.collision_query_, players, match[1].str(), match[2].str());
            players.add_player(*player);
            physics_engine.advance_times_.add_advance_time(player);
            physics_engine.add_external_force_provider(player.get());
        } else if (std::regex_match(line, match, player_set_node_reg)) {
            auto node = scene.get_node(match[2].str());
            auto rb = dynamic_cast<RigidBody*>(node->get_absolute_movable());
            if (rb == nullptr) {
                throw std::runtime_error("Follower movable is not a rigid body");
            }
            players.get_player(match[1].str()).set_rigid_body(*node, *rb);
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
            players.get_player(match[1].str()).set_tire_angle(
                safe_stoi(match[2].str()),
                safe_stof(match[3].str()),
                safe_stof(match[4].str()));
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
            camera_key_bindings.push_back(CameraKeyBinding{
                base: {
                    key: match[1].str(),
                    gamepad_button: match[2].str(),
                    joystick_axis: match[3].str(),
                    joystick_axis_sign: safe_stof(match[4].str())}});
        } else if (std::regex_match(line, match, abs_idle_binding_reg)) {
            absolute_movable_idle_bindings.push_back(AbsoluteMovableIdleBinding{
                node: match[1].str(),
                tires_z: {
                    match[2].str().empty() ? 0 : safe_stof(match[2].str()),
                    match[3].str().empty() ? 0 : safe_stof(match[3].str()),
                    match[4].str().empty() ? 1 : safe_stof(match[4].str())}});
        } else if (std::regex_match(line, match, abs_key_binding_reg)) {
            absolute_movable_key_bindings.push_back(AbsoluteMovableKeyBinding{
                base_key: {
                    key: match[2].str(),
                    gamepad_button: match[3].str(),
                    joystick_axis: match[4].str(),
                    joystick_axis_sign: match[5].str().empty() ? 0 : safe_stof(match[5].str())},
                node: match[1].str(),
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
                tire_angle: match[18].str().empty() ? NAN : safe_stof(match[18].str()),
                tires_z: {
                    match[19].str().empty() ? 0 : safe_stof(match[19].str()),
                    match[20].str().empty() ? 0 : safe_stof(match[20].str()),
                    match[21].str().empty() ? 0 : safe_stof(match[21].str())}});
        } else if (std::regex_match(line, match, rel_key_binding_reg)) {
            relative_movable_key_bindings.push_back(RelativeMovableKeyBinding{
                base_key: {
                    key: match[2].str(),
                    gamepad_button: match[3].str(),
                    joystick_axis: match[4].str(),
                    joystick_axis_sign: safe_stof(match[5].str())},
                node: match[1].str(),
                angular_velocity_press: {
                    safe_stof(match[6].str()),
                    safe_stof(match[7].str()),
                    safe_stof(match[8].str())},
                angular_velocity_repeat: {
                    safe_stof(match[9].str()),
                    safe_stof(match[10].str()),
                    safe_stof(match[11].str())}});
        } else if (std::regex_match(line, match, gun_key_binding_reg)) {
            gun_key_bindings.push_back(GunKeyBinding{
                base: {
                    key: match[2].str(),
                    gamepad_button: match[3].str(),
                    joystick_axis: match[4].str(),
                    joystick_axis_sign: match[5].str().empty() ? 0 : safe_stof(match[5].str())},
                node: match[1].str()});
        } else if (std::regex_match(line, match, console_log_reg)) {
            auto node = scene.get_node(match[1].str());
            auto mv = node->get_absolute_movable();
            auto lo = dynamic_cast<Loggable*>(mv);
            if (lo == nullptr) {
                throw std::runtime_error("Could not find loggable");
            }
            unsigned int log_components = safe_stoi(match[2].str());
            auto logger = std::make_shared<MovableLogger>(*node, physics_engine.advance_times_, lo, log_components);
            physics_engine.advance_times_.add_advance_time(logger);
        } else if (std::regex_match(line, match, visual_log_reg)) {
            auto node = scene.get_node(match[1].str());
            auto mv = node->get_absolute_movable();
            auto lo = dynamic_cast<Loggable*>(mv);
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
        } else if (std::regex_match(line, match, visual_log_3rd_reg)) {
            auto node = scene.get_node(match[1].str());
            auto mv = node->get_absolute_movable();
            auto lo = dynamic_cast<Loggable*>(mv);
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
            for(const auto& e : find_all_name_values(match[7].str(), "[\\w-. \\(\\)/+-]+")) {
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
            for(const auto& e : find_all_name_values(match[7].str(), substitute_pattern)) {
                rps.push_back(ReplacementParameter{
                    name: e.first,
                    substitutions: SubstitutionString{e.second}});
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
                selection_ids[match[1].str()]);
            render_logics.append(nullptr, parameter_setter_logic);
        } else if (std::regex_match(line, match, ui_background_reg)) {
            auto bg = std::make_shared<MainMenuBackgroundLogic>(
                rendering_resources,
                fpath(match[1].str()),
                ui_focus.focus,
                focus_from_string(match[2].str()));
            render_logics.append(nullptr, bg);
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
            auto node = scene.get_node(match[1].str());
            size_t resource_index = selected_cameras.add_light_node(match[1].str());
            render_logics.prepend(node, std::make_shared<LightmapLogic>(
                read_pixels_logic,
                rendering_resources,
                lightmap_update_cycle_from_string(match[3].str()),
                resource_index,
                match[2].str(),               // black_node_name
                safe_stob(match[4].str())));  // with_depth_texture
            node->add_light(new Light{
                ambience: {
                    safe_stof(match[5].str()),
                    safe_stof(match[6].str()),
                    safe_stof(match[7].str())},
                diffusivity: {
                    safe_stof(match[8].str()),
                    safe_stof(match[9].str()),
                    safe_stof(match[10].str())},
                specularity: {
                    safe_stof(match[11].str()),
                    safe_stof(match[12].str()),
                    safe_stof(match[13].str())},
                resource_index: resource_index,
                only_black: !match[2].str().empty()});
        } else if (std::regex_match(line, match, look_at_node_reg)) {
            auto follower_node = scene.get_node(match[1].str());
            auto followed_node = scene.get_node(match[2].str());
            auto follower = std::make_shared<LookAtMovable>(
                physics_engine.advance_times_,
                followed_node,
                followed_node->get_absolute_movable());
            linker.link_absolute_movable(*follower_node, follower);
        } else if (std::regex_match(line, match, keep_offset_reg)) {
            auto follower_node = scene.get_node(match[1].str());
            auto followed_node = scene.get_node(match[2].str());
            auto follower = std::make_shared<KeepOffsetMovable>(
                physics_engine.advance_times_,
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
                safe_stof(match[12].str()));       // y_snappiness
            linker.link_absolute_movable(*follower_node, follower);
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
            auto beacon_node0 = scene.get_node(match[2].str());
            auto beacon_node1 = scene.get_node(match[3].str());
            physics_engine.advance_times_.add_advance_time(std::make_shared<CheckPoints>(
                fpath(match[7].str()),                  // filename
                physics_engine.advance_times_,
                moving_node,
                moving_node->get_absolute_movable(),
                beacon_node0,
                beacon_node1,
                &players,
                &players.get_player(match[4].str()),
                safe_stoi(match[5].str()),              // nth
                safe_stof(match[6].str())));            // radius
        } else if (std::regex_match(line, match, set_camera_cycle_reg)) {
            std::regex re{"\\s+"};
            std::string cameras = match[2].str();
            auto& cycle = (match[1].str() == "near")
                ? selected_cameras.camera_cycle_near
                : selected_cameras.camera_cycle_far;
            cycle = string_to_vector(cameras);
        } else if (std::regex_match(line, match, set_camera_reg)) {
            selected_cameras.camera_node_name = match[1].str();
        } else if (std::regex_match(line, match, set_dirtmap_reg)) {
            dirtmap_logic.set_filename(fpath(match[1].str()));
        } else if (std::regex_match(line, match, set_skybox_reg)) {
            skybox_logic.set_filenames({
                fpath(match[2].str()),
                fpath(match[3].str()),
                fpath(match[4].str()),
                fpath(match[5].str()),
                fpath(match[6].str()),
                fpath(match[7].str())},
                match[1].str());
        } else if (std::regex_match(line, match, burn_in_reg)) {
            physics_engine.burn_in(safe_stof(match[1].str()));
        } else if (std::regex_match(line, match, append_focus_reg)) {
            ui_focus.focus.push_back(focus_from_string(match[1].str()));
        } else if (std::regex_match(line, match, macro_playback_reg)) {
            auto macro_it = macros_.find(match[1].str());
            if (macro_it == macros_.end()) {
                throw std::runtime_error("No macro with name " + match[1].str() + " exists");
            }
            for(const std::string& l : macro_it->second.lines) {
                process_line(substitute(l, match[2].str()), macro_it->second.filename);
            }
        } else if (std::regex_match(line, match, include_reg)) {
            (*this)(
                scene_filename,
                spath(match[1].str()),
                next_scene_filename,
                rendering_resources,
                scene_node_resources,
                players,
                scene,
                physics_engine,
                button_press,
                camera_key_bindings,
                absolute_movable_idle_bindings,
                absolute_movable_key_bindings,
                relative_movable_key_bindings,
                gun_key_bindings,
                selected_cameras,
                camera_config,
                physics_engine_config,
                render_logics,
                scene_logic,
                read_pixels_logic,
                dirtmap_logic,
                skybox_logic,
                ui_focus,
                substitutions,
                num_renderings,
                selection_ids,
                verbose);
        } else {
            throw std::runtime_error("Could not parse line: \"" + line + '"');
        }
    };
    std::string line;
    std::list<std::pair<std::string, Macro>> recording_macros;
    while(std::getline(ifs, line, ';')) {
        if (line.length() > 0 && line[line.length() - 1] == '\r') {
            line = line.substr(0, line.length() - 1);
        }
        std::smatch match;

        if (std::regex_match(line, match, macro_begin_reg)) {
            recording_macros.push_back(std::make_pair(match[1].str(), Macro{filename: script_filename}));
        } else if (std::regex_match(line, match, macro_end_reg)) {
            if (recording_macros.empty()) {
                throw std::runtime_error("Macro-end despite no active macro");
            }
            auto it = macros_.insert(std::make_pair(recording_macros.back().first, recording_macros.back().second));
            if (!it.second) {
                throw std::runtime_error("Duplicate macro: " + recording_macros.back().first);
            }
            recording_macros.pop_back();
        } else if (!recording_macros.empty()) {
            recording_macros.back().second.lines.push_back(line);
        } else {
            process_line(substitutions.substitute(line), script_filename);
        }
    }

    if (!ifs.eof() && ifs.fail()) {
        throw std::runtime_error("Error reading from file: \"" + script_filename + '"');
    }
}
