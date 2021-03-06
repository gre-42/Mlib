#include "Obj_Resource.hpp"
#include <Mlib/FPath.hpp>
#include <Mlib/Geometry/Material/Blend_Mode.hpp>
#include <Mlib/Geometry/Mesh/Load_Mesh_Config.hpp>
#include <Mlib/Physics/Units.hpp>
#include <Mlib/Regex_Select.hpp>
#include <Mlib/Render/Resources/Mhx2_File_Resource.hpp>
#include <Mlib/Render/Resources/Obj_File_Resource.hpp>
#include <Mlib/Scene/User_Function_Args.hpp>
#include <Mlib/Scene_Graph/Aggregate_Mode.hpp>
#include <Mlib/Scene_Graph/Render_Pass.hpp>
#include <Mlib/Scene_Graph/Scene_Node_Resources.hpp>
#include <Mlib/Scene_Graph/Transformation_Mode.hpp>

using namespace Mlib;

#define BEGIN_OPTIONS static size_t option_id = 1
#define DECLARE_OPTION(a) static const size_t a = option_id++

BEGIN_OPTIONS;
DECLARE_OPTION(NAME);
DECLARE_OPTION(FILENAME);
DECLARE_OPTION(POSITION_X);
DECLARE_OPTION(POSITION_Y);
DECLARE_OPTION(POSITION_Z);
DECLARE_OPTION(ROTATION_X);
DECLARE_OPTION(ROTATION_Y);
DECLARE_OPTION(ROTATION_Z);
DECLARE_OPTION(SCALE_X);
DECLARE_OPTION(SCALE_Y);
DECLARE_OPTION(SCALE_Z);
DECLARE_OPTION(DISTANCE_NEAR);
DECLARE_OPTION(DISTANCE_FAR);
DECLARE_OPTION(BLEND_MODE);
DECLARE_OPTION(ALPHA_DISTANCES_0);
DECLARE_OPTION(ALPHA_DISTANCES_1);
DECLARE_OPTION(ALPHA_DISTANCES_2);
DECLARE_OPTION(ALPHA_DISTANCES_3);
DECLARE_OPTION(CULL_FACES_DEFAULT);
DECLARE_OPTION(CULL_FACES_ALPHA);
DECLARE_OPTION(OCCLUDED_PASS);
DECLARE_OPTION(OCCLUDER_PASS);
DECLARE_OPTION(AGGREGATE_MODE);
DECLARE_OPTION(TRANSFORMATION_MODE);
DECLARE_OPTION(REFLECTION_MAP);
DECLARE_OPTION(TRIANGLE_TANGENT_ERROR_BEHAVIOR);
DECLARE_OPTION(NO_WERROR);

LoadSceneUserFunction ObjResource::user_function = [](const LoadSceneUserFunctionArgs& args)
{
    static DECLARE_REGEX(regex,
        "^\\s*obj_resource"
        "\\s+name=([\\w+-. \\(\\)/]+)"
        "\\s+filename=([\\w+-. \\(\\)/\\\\:]+)"
        "\\s+position=([\\w+-.]+)\\s+([\\w+-.]+)\\s+([\\w+-.]+)"
        "\\s+rotation=([\\w+-.]+)\\s+([\\w+-.]+)\\s+([\\w+-.]+)"
        "\\s+scale=([\\w+-.]+)\\s+([\\w+-.]+)\\s+([\\w+-.]+)"
        "(?:\\s+center_distances=([\\w+-.]+)\\s+([\\w+-.]+))?"
        "\\s+blend_mode=(\\w+)"
        "\\s+alpha_distances=([\\w+-.]+)\\s+([\\w+-.]+)\\s+([\\w+-.]+)\\s+([\\w+-.]+)"
        "(?:\\s+cull_faces_default=(0|1))?"
        "(?:\\s+cull_faces_alpha=(0|1))?"
        "\\s+occluded_pass=(\\w+)"
        "\\s+occluder_pass=(\\w+)"
        "\\s+aggregate_mode=(\\w+)"
        "\\s+transformation_mode=(\\w+)"
        "(?:\\s+reflection_map=(\\w*))?"
        "(?:\\s+triangle_tangent_error_behavior=(zero|warn|raise))?"
        "(\\s+no_werror)?$");
    std::smatch match;
    if (Mlib::re::regex_match(args.line, match, regex)) {
        execute(match, args);
        return true;
    } else {
        return false;
    }
};

void ObjResource::execute(
    const Mlib::re::smatch& match,
    const LoadSceneUserFunctionArgs& args)
{
    LoadMeshConfig load_mesh_config{
        .position = FixedArray<float, 3>{
            safe_stof(match[POSITION_X].str()),
            safe_stof(match[POSITION_Y].str()),
            safe_stof(match[POSITION_Z].str())},
        .rotation = FixedArray<float, 3>{
            safe_stof(match[ROTATION_X].str()) * degrees,
            safe_stof(match[ROTATION_Y].str()) * degrees,
            safe_stof(match[ROTATION_Z].str()) * degrees},
        .scale = FixedArray<float, 3>{
            safe_stof(match[SCALE_X].str()),
            safe_stof(match[SCALE_Y].str()),
            safe_stof(match[SCALE_Z].str())},
        .center_distances = OrderableFixedArray<float, 2>{
            match[DISTANCE_NEAR].matched ? safe_stof(match[DISTANCE_NEAR].str()) : 0.f,
            match[DISTANCE_FAR].matched ? safe_stof(match[DISTANCE_FAR].str()) : float { INFINITY }},
        .blend_mode = blend_mode_from_string(match[BLEND_MODE].str()),
        .alpha_distances = {
            safe_stof(match[ALPHA_DISTANCES_0].str()),
            safe_stof(match[ALPHA_DISTANCES_1].str()),
            safe_stof(match[ALPHA_DISTANCES_2].str()),
            safe_stof(match[ALPHA_DISTANCES_3].str())},
        .cull_faces_default = match[CULL_FACES_DEFAULT].matched
            ? safe_stob(match[CULL_FACES_DEFAULT].str())
            : true,
        .cull_faces_alpha = match[CULL_FACES_ALPHA].matched
            ? safe_stob(match[CULL_FACES_ALPHA].str())
            : true,
        .occluded_pass = external_render_pass_type_from_string(match[OCCLUDED_PASS].str()),
        .occluder_pass = external_render_pass_type_from_string(match[OCCLUDER_PASS].str()),
        .aggregate_mode = aggregate_mode_from_string(match[AGGREGATE_MODE].str()),
        .transformation_mode = transformation_mode_from_string(match[TRANSFORMATION_MODE].str()),
        .reflection_map = match[REFLECTION_MAP].str(),
        .triangle_tangent_error_behavior = match[TRIANGLE_TANGENT_ERROR_BEHAVIOR].matched
            ? triangle_tangent_error_behavior_from_string(match[TRIANGLE_TANGENT_ERROR_BEHAVIOR].str())
            : TriangleTangentErrorBehavior::RAISE,
        .apply_static_lighting = false,
        .werror = !match[NO_WERROR].matched};
    std::string filename = args.fpath(match[FILENAME].str()).path;
    if (filename.ends_with(".obj")) {
        args.scene_node_resources.add_resource_loader(
            match[NAME].str(),
            [filename, load_mesh_config, &scene_node_resources=args.scene_node_resources](){
                return std::make_shared<ObjFileResource>(
                    filename,
                    load_mesh_config,
                    scene_node_resources);
            });
    } else if (filename.ends_with(".mhx2")) {
        args.scene_node_resources.add_resource_loader(
            match[NAME].str(),
            [filename, load_mesh_config](){
                return std::make_shared<Mhx2FileResource>(
                    filename,
                    load_mesh_config);
            });
    } else {
        throw std::runtime_error("Unknown file type: " + filename);
    }
}
