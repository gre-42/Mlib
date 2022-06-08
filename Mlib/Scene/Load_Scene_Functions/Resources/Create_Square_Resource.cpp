#include "Create_Square_Resource.hpp"
#include <Mlib/FPath.hpp>
#include <Mlib/Geometry/Material.hpp>
#include <Mlib/Geometry/Material/Billboard_Atlas_Instance.hpp>
#include <Mlib/Math/Fixed_Rodrigues.hpp>
#include <Mlib/Math/Transformation_Matrix.hpp>
#include <Mlib/Physics/Units.hpp>
#include <Mlib/Regex.hpp>
#include <Mlib/Regex_Select.hpp>
#include <Mlib/Render/Resources/Square_Resource.hpp>
#include <Mlib/Scene/User_Function_Args.hpp>
#include <Mlib/Scene_Graph/Scene_Node_Resources.hpp>
#include <vector>

using namespace Mlib;

#define BEGIN_OPTIONS static size_t option_id = 1
#define DECLARE_OPTION(a) static const size_t a = option_id++

BEGIN_OPTIONS;
DECLARE_OPTION(NAME);
DECLARE_OPTION(TEXTURE_FILENAME);
DECLARE_OPTION(MIN_X);
DECLARE_OPTION(MIN_Y);
DECLARE_OPTION(MAX_X);
DECLARE_OPTION(MAX_Y);
DECLARE_OPTION(DISTANCES_0);
DECLARE_OPTION(DISTANCES_1);
DECLARE_OPTION(IS_SMALL);
DECLARE_OPTION(OCCLUDED_PASS);
DECLARE_OPTION(OCCLUDER_PASS);
DECLARE_OPTION(EMISSIVITY_R);
DECLARE_OPTION(EMISSIVITY_G);
DECLARE_OPTION(EMISSIVITY_B);
DECLARE_OPTION(AMBIENCE_R);
DECLARE_OPTION(AMBIENCE_G);
DECLARE_OPTION(AMBIENCE_B);
DECLARE_OPTION(BLEND_MODE);
DECLARE_OPTION(DEPTH_FUNC);
DECLARE_OPTION(ALPHA_DISTANCES_0);
DECLARE_OPTION(ALPHA_DISTANCES_1);
DECLARE_OPTION(ALPHA_DISTANCES_2);
DECLARE_OPTION(ALPHA_DISTANCES_3);
DECLARE_OPTION(CULL_FACES);
DECLARE_OPTION(ROTATION_X);
DECLARE_OPTION(ROTATION_Y);
DECLARE_OPTION(ROTATION_Z);
DECLARE_OPTION(TRANSLATION_X);
DECLARE_OPTION(TRANSLATION_Y);
DECLARE_OPTION(TRANSLATION_Z);
DECLARE_OPTION(AGGREGATE_MODE);
DECLARE_OPTION(TRANSFORMATION_MODE);
DECLARE_OPTION(NUMBER_OF_FRAMES);
DECLARE_OPTION(BILLBOARDS);

LoadSceneUserFunction CreateSquareResource::user_function = [](const LoadSceneUserFunctionArgs& args)
{
    static DECLARE_REGEX(regex,
        "^\\s*square_resource"
        "\\s+name=([\\w+-.]+)"
        "\\s+texture_filename=(#?[\\w+-.\\(\\)/]+)"
        "\\s+min=([\\w+-.]+)\\s+([\\w+-.]+)"
        "\\s+max=([\\w+-.]+)\\s+([\\w+-.]+)"
        "(?:\\s+distances=([\\w+-.]+)\\s+([\\w+-.]+))?"
        "\\s+is_small=(0|1)"
        "\\s+occluded_pass=(\\w+)"
        "\\s+occluder_pass=(\\w+)"
        "(?:\\s+emissivity=([\\w+-.]+)\\s+([\\w+-.]+)\\s+([\\w+-.]+))?"
        "\\s+ambience=([\\w+-.]+)\\s+([\\w+-.]+)\\s+([\\w+-.]+)"
        "\\s+blend_mode=(\\w+)"
        "(?:\\s+depth_func=(\\w+))?"
        "\\s+alpha_distances=([\\w+-.]+)\\s+([\\w+-.]+)\\s+([\\w+-.]+)\\s+([\\w+-.]+)"
        "\\s+cull_faces=(0|1)"
        "(?:\\s+rotation=([\\w+-.]+)\\s+([\\w+-.]+)\\s+([\\w+-.]+))?"
        "(?:\\s+translation=([\\w+-.]+)\\s+([\\w+-.]+)\\s+([\\w+-.]+))?"
        "\\s+aggregate_mode=(\\w+)"
        "\\s+transformation_mode=(\\w+)"
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
    const Mlib::re::smatch& match,
    const LoadSceneUserFunctionArgs& args)
{
    std::vector<BillboardAtlasInstance> billboard_atlas_instances;
    static const DECLARE_REGEX(
        street_texture_reg,
        "(?:\\s*uv_scale:\\s*([\\w+-.]+)\\s+([\\w+-.]+)"
        "\\s+uv_offset:\\s*([\\w+-.]+)\\s+([\\w+-.]+)"
        "\\s+vertex_scale:\\s*([\\w+-.]+)\\s+([\\w+-.]+)"
        "\\s+is_small:(0|1)"
        "\\s+occluder_pass:(\\w+)"
        "|([\\s\\S]+))");
    find_all(match[BILLBOARDS].str(), street_texture_reg, [&](const Mlib::re::smatch& match3) {
        if (match3[9].matched) {
            throw std::runtime_error("Unknown element: \"" + match3[9].str() + '"');
        }
        billboard_atlas_instances.push_back(BillboardAtlasInstance{
            .uv_scale = OrderableFixedArray<float, 2>{safe_stof(match3[1].str()), safe_stof(match3[2].str())},
            .uv_offset = OrderableFixedArray<float, 2>{safe_stof(match3[3].str()), safe_stof(match3[4].str())},
            .vertex_scale = OrderableFixedArray<float, 2>{safe_stof(match3[5].str()), safe_stof(match3[6].str())},
            .is_small = safe_stob(match3[7].str()),
            .occluder_pass = external_render_pass_type_from_string(match3[8].str())});
    });
    args.scene_node_resources.add_resource(match[NAME].str(), std::make_shared<SquareResource>(
        FixedArray<float, 2, 2>{
            safe_stof(match[MIN_X].str()), safe_stof(match[MIN_Y].str()),
            safe_stof(match[MAX_X].str()), safe_stof(match[MAX_Y].str())},
        TransformationMatrix<float, 3>(
            tait_bryan_angles_2_matrix(
                FixedArray<float, 3>{
                    match[ROTATION_X].matched ? safe_stof(match[ROTATION_X].str()) * degrees : 0.f,
                    match[ROTATION_Y].matched ? safe_stof(match[ROTATION_Y].str()) * degrees : 0.f,
                    match[ROTATION_Z].matched ? safe_stof(match[ROTATION_Z].str()) * degrees : 0.f}),
            FixedArray<float, 3>{
                match[TRANSLATION_X].matched ? safe_stof(match[TRANSLATION_X].str()) : 0.f,
                match[TRANSLATION_Y].matched ? safe_stof(match[TRANSLATION_Y].str()) : 0.f,
                match[TRANSLATION_Z].matched ? safe_stof(match[TRANSLATION_Z].str()) : 0.f}),
        Material{
            .blend_mode = blend_mode_from_string(match[BLEND_MODE].str()),
            .depth_func = match[DEPTH_FUNC].matched ? depth_func_from_string(match[DEPTH_FUNC].str()) : DepthFunc::LESS,
            .textures = {{.texture_descriptor = {.color = args.fpath(match[TEXTURE_FILENAME].str()).path}}},
            .occluded_pass = external_render_pass_type_from_string(match[OCCLUDED_PASS].str()),
            .occluder_pass = external_render_pass_type_from_string(match[OCCLUDER_PASS].str()),
            .alpha_distances = {
                safe_stof(match[ALPHA_DISTANCES_0].str()),
                safe_stof(match[ALPHA_DISTANCES_1].str()),
                safe_stof(match[ALPHA_DISTANCES_2].str()),
                safe_stof(match[ALPHA_DISTANCES_3].str())},
            .wrap_mode_s = WrapMode::CLAMP_TO_EDGE,
            .wrap_mode_t = WrapMode::CLAMP_TO_EDGE,
            .aggregate_mode = aggregate_mode_from_string(match[AGGREGATE_MODE].str()),
            .transformation_mode = transformation_mode_from_string(match[TRANSFORMATION_MODE].str()),
            .billboard_atlas_instances = billboard_atlas_instances,
            .number_of_frames = match[NUMBER_OF_FRAMES].matched ? safe_stou(match[NUMBER_OF_FRAMES].str()) : 1,
            .distances = OrderableFixedArray<float, 2>{
                match[DISTANCES_0].matched ? safe_stof(match[DISTANCES_0].str()) : 0.f,
                match[DISTANCES_1].matched ? safe_stof(match[DISTANCES_1].str()) : float { INFINITY }},
            .is_small = safe_stob(match[IS_SMALL].str()),
            .cull_faces = safe_stob(match[CULL_FACES].str()),
            .emissivity = {
                match[EMISSIVITY_R].matched ? safe_stof(match[EMISSIVITY_R].str()) : 0.f,
                match[EMISSIVITY_G].matched ? safe_stof(match[EMISSIVITY_G].str()) : 0.f,
                match[EMISSIVITY_B].matched ? safe_stof(match[EMISSIVITY_B].str()) : 0.f},
            .ambience = {
                safe_stof(match[AMBIENCE_R].str()),
                safe_stof(match[AMBIENCE_G].str()),
                safe_stof(match[AMBIENCE_B].str())},
            .diffusivity = {0.f, 0.f, 0.f},
            .specularity = {0.f, 0.f, 0.f}}.compute_color_mode()));

}
