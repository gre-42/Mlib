#include "Obj_Resource.hpp"
#include <Mlib/Geometry/Material/Blend_Mode.hpp>
#include <Mlib/Geometry/Material/Occluded_Type.hpp>
#include <Mlib/Geometry/Material/Occluder_Type.hpp>
#include <Mlib/Geometry/Mesh/Load_Mesh_Config.hpp>
#include <Mlib/Macro_Line_Executor.hpp>
#include <Mlib/Regex_Select.hpp>
#include <Mlib/Render/Resources/Mhx2_File_Resource.hpp>
#include <Mlib/Render/Resources/Obj_File_Resource.hpp>
#include <Mlib/Scene/User_Function_Args.hpp>
#include <Mlib/Scene_Graph/Aggregate_Mode.hpp>
#include <Mlib/Scene_Graph/Scene_Node_Resources.hpp>
#include <Mlib/Scene_Graph/Transformation_Mode.hpp>

using namespace Mlib;

LoadSceneUserFunction ObjResource::user_function = [](const LoadSceneUserFunctionArgs& args)
{
    static DECLARE_REGEX(regex,
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
    std::smatch match;
    if (Mlib::re::regex_match(args.line, match, regex)) {
        execute(match, args);
        return true;
    } else {
        return false;
    }
};

void ObjResource::execute(
    const std::smatch& match,
    const LoadSceneUserFunctionArgs& args)
{
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
    std::string filename = args.fpath(match[2].str()).path;
    if (filename.ends_with(".obj")) {
        args.scene_node_resources.add_resource_loader(
            match[1].str(),
            [filename, load_mesh_config](){
                return std::make_shared<ObjFileResource>(
                    filename,
                    load_mesh_config);
            });
    } else if (filename.ends_with(".mhx2")) {
        args.scene_node_resources.add_resource_loader(
            match[1].str(),
            [filename, load_mesh_config](){
                return std::make_shared<Mhx2FileResource>(
                    filename,
                    load_mesh_config);
            });
    } else {
        throw std::runtime_error("Unknown file type: " + filename);
    }
}
