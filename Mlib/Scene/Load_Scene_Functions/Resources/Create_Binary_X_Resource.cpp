#include "Create_Binary_X_Resource.hpp"
#include <Mlib/FPath.hpp>
#include <Mlib/Geometry/Material.hpp>
#include <Mlib/Regex_Select.hpp>
#include <Mlib/Render/Resources/Binary_X_Resource.hpp>
#include <Mlib/Scene/User_Function_Args.hpp>
#include <Mlib/Scene_Graph/Scene_Node_Resources.hpp>

using namespace Mlib;

LoadSceneUserFunction CreateBinaryXResource::user_function = [](const LoadSceneUserFunctionArgs& args)
{
    static DECLARE_REGEX(regex,
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
    std::smatch match;
    if (Mlib::re::regex_match(args.line, match, regex)) {
        execute(match, args);
        return true;
    } else {
        return false;
    }
};

void CreateBinaryXResource::execute(
    const std::smatch& match,
    const LoadSceneUserFunctionArgs& args)
{
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
    material_0.textures = {{.texture_descriptor = {.color = args.fpath(match[2].str()).path}}};
    material_90.textures = {{.texture_descriptor = {.color = args.fpath(match[3].str()).path}}};
    material_0.compute_color_mode();
    material_90.compute_color_mode();
    args.scene_node_resources.add_resource(match[1].str(), std::make_shared<BinaryXResource>(
        FixedArray<float, 2, 2>{
            safe_stof(match[4].str()), safe_stof(match[5].str()),
            safe_stof(match[6].str()), safe_stof(match[7].str())},
        material_0,
        material_90));
}
