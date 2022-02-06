#include "Create_Square_Resource.hpp"
#include <Mlib/FPath.hpp>
#include <Mlib/Geometry/Material.hpp>
#include <Mlib/Geometry/Material/Billboard_Atlas_Instance.hpp>
#include <Mlib/Math/Fixed_Rodrigues.hpp>
#include <Mlib/Math/Transformation_Matrix.hpp>
#include <Mlib/Regex.hpp>
#include <Mlib/Regex_Select.hpp>
#include <Mlib/Render/Resources/Square_Resource.hpp>
#include <Mlib/Scene/User_Function_Args.hpp>
#include <Mlib/Scene_Graph/Scene_Node_Resources.hpp>
#include <vector>

using namespace Mlib;

LoadSceneUserFunction CreateSquareResource::user_function = [](const LoadSceneUserFunctionArgs& args)
{
    static DECLARE_REGEX(regex,
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
    std::smatch match;
    if (Mlib::re::regex_match(args.line, match, regex)) {
        execute(match, args);
        return true;
    } else {
        return false;
    }
};

void CreateSquareResource::execute(
    const std::smatch& match,
    const LoadSceneUserFunctionArgs& args)
{
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
    args.scene_node_resources.add_resource(match[1].str(), std::make_shared<SquareResource>(
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
            .textures = {{.texture_descriptor = {.color = args.fpath(match[2].str()).path}}},
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

}
