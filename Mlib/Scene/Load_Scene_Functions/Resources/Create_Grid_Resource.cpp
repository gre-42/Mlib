#include "Create_Grid_Resource.hpp"
#include <Mlib/FPath.hpp>
#include <Mlib/Geometry/Material.hpp>
#include <Mlib/Math/Fixed_Rodrigues.hpp>
#include <Mlib/Math/Transformation_Matrix.hpp>
#include <Mlib/Physics/Units.hpp>
#include <Mlib/Regex.hpp>
#include <Mlib/Regex_Select.hpp>
#include <Mlib/Render/Rendering_Context.hpp>
#include <Mlib/Render/Rendering_Resources.hpp>
#include <Mlib/Render/Resources/Grid_Resource.hpp>
#include <Mlib/Scene/User_Function_Args.hpp>
#include <Mlib/Scene_Graph/Scene_Node_Resources.hpp>
#include <vector>

using namespace Mlib;

#define BEGIN_OPTIONS static size_t option_id = 1
#define DECLARE_OPTION(a) static const size_t a = option_id++

BEGIN_OPTIONS;
DECLARE_OPTION(NAME);
DECLARE_OPTION(TEXTURE_FILENAME);
DECLARE_OPTION(SIZE_X);
DECLARE_OPTION(SIZE_Y);
DECLARE_OPTION(CENTER_DISTANCES_0);
DECLARE_OPTION(CENTER_DISTANCES_1);
DECLARE_OPTION(OCCLUDED_PASS);
DECLARE_OPTION(OCCLUDER_PASS);
DECLARE_OPTION(EMISSIVITY_R);
DECLARE_OPTION(EMISSIVITY_G);
DECLARE_OPTION(EMISSIVITY_B);
DECLARE_OPTION(AMBIENCE_R);
DECLARE_OPTION(AMBIENCE_G);
DECLARE_OPTION(AMBIENCE_B);
DECLARE_OPTION(DIFFUSIVITY_R);
DECLARE_OPTION(DIFFUSIVITY_G);
DECLARE_OPTION(DIFFUSIVITY_B);
DECLARE_OPTION(SPECULARITY_R);
DECLARE_OPTION(SPECULARITY_G);
DECLARE_OPTION(SPECULARITY_B);
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
DECLARE_OPTION(SCALE);
DECLARE_OPTION(UV_SCALE);
DECLARE_OPTION(PERIOD);
DECLARE_OPTION(AGGREGATE_MODE);
DECLARE_OPTION(TRANSFORMATION_MODE);

LoadSceneUserFunction CreateGridResource::user_function = [](const LoadSceneUserFunctionArgs& args)
{
    static DECLARE_REGEX(regex,
        "^\\s*grid_resource"
        "\\s+name=([\\w+-.]+)"
        "\\s+texture_filename=(#?[\\w+-.\\(\\)/]+)"
        "\\s+size=([\\w+-.]+)\\s+([\\w+-.]+)"
        "(?:\\s+center_distances=([\\w+-.]+)\\s+([\\w+-.]+))?"
        "\\s+occluded_pass=(\\w+)"
        "\\s+occluder_pass=(\\w+)"
        "(?:\\s+emissivity=([\\w+-.]+)\\s+([\\w+-.]+)\\s+([\\w+-.]+))?"
        "\\s+ambience=([\\w+-.]+)\\s+([\\w+-.]+)\\s+([\\w+-.]+)"
        "\\s+diffusivity=([\\w+-.]+)\\s+([\\w+-.]+)\\s+([\\w+-.]+)"
        "\\s+specularity=([\\w+-.]+)\\s+([\\w+-.]+)\\s+([\\w+-.]+)"
        "\\s+blend_mode=(\\w+)"
        "(?:\\s+depth_func=(\\w+))?"
        "\\s+alpha_distances=([\\w+-.]+)\\s+([\\w+-.]+)\\s+([\\w+-.]+)\\s+([\\w+-.]+)"
        "\\s+cull_faces=(0|1)"
        "(?:\\s+rotation=([\\w+-.]+)\\s+([\\w+-.]+)\\s+([\\w+-.]+))?"
        "(?:\\s+translation=([\\w+-.]+)\\s+([\\w+-.]+)\\s+([\\w+-.]+))?"
        "\\s+scale=([\\w+-.]+)"
        "\\s+uv_scale=([\\w+-.]+)"
        "\\s+period=([\\w+-.]+)"
        "\\s+aggregate_mode=(\\w+)"
        "\\s+transformation_mode=(\\w+)$");
    Mlib::re::smatch match;
    if (Mlib::re::regex_match(args.line, match, regex)) {
        execute(match, args);
        return true;
    } else {
        return false;
    }
};

void CreateGridResource::execute(
    const Mlib::re::smatch& match,
    const LoadSceneUserFunctionArgs& args)
{
    auto primary_rendering_resources = RenderingContextStack::primary_rendering_resources();

    args.scene_node_resources.add_resource(match[NAME].str(), std::make_shared<GridResource>(
        FixedArray<size_t, 2>{
            safe_stoz(match[SIZE_X].str()),
            safe_stoz(match[SIZE_Y].str())},
        TransformationMatrix<float, double, 3>(
            tait_bryan_angles_2_matrix(
                FixedArray<float, 3>{
                    match[ROTATION_X].matched ? safe_stof(match[ROTATION_X].str()) * degrees : 0.f,
                    match[ROTATION_Y].matched ? safe_stof(match[ROTATION_Y].str()) * degrees : 0.f,
                    match[ROTATION_Z].matched ? safe_stof(match[ROTATION_Z].str()) * degrees : 0.f}),
            FixedArray<double, 3>{
                match[TRANSLATION_X].matched ? safe_stod(match[TRANSLATION_X].str()) : 0.,
                match[TRANSLATION_Y].matched ? safe_stod(match[TRANSLATION_Y].str()) : 0.,
                match[TRANSLATION_Z].matched ? safe_stod(match[TRANSLATION_Z].str()) : 0.}),
        safe_stod(match[SCALE].str()),
        safe_stod(match[UV_SCALE].str()),
        safe_stod(match[PERIOD].str()),
        Material{
            .blend_mode = blend_mode_from_string(match[BLEND_MODE].str()),
            .depth_func = match[DEPTH_FUNC].matched ? depth_func_from_string(match[DEPTH_FUNC].str()) : DepthFunc::LESS,
            .textures = {primary_rendering_resources->get_blend_map_texture(args.fpath(match[TEXTURE_FILENAME].str()).path)},
            .occluded_pass = external_render_pass_type_from_string(match[OCCLUDED_PASS].str()),
            .occluder_pass = external_render_pass_type_from_string(match[OCCLUDER_PASS].str()),
            .alpha_distances = {
                safe_stof(match[ALPHA_DISTANCES_0].str()),
                safe_stof(match[ALPHA_DISTANCES_1].str()),
                safe_stof(match[ALPHA_DISTANCES_2].str()),
                safe_stof(match[ALPHA_DISTANCES_3].str())},
            .wrap_mode_s = WrapMode::REPEAT,
            .wrap_mode_t = WrapMode::REPEAT,
            .aggregate_mode = aggregate_mode_from_string(match[AGGREGATE_MODE].str()),
            .transformation_mode = transformation_mode_from_string(match[TRANSFORMATION_MODE].str()),
            .center_distances = OrderableFixedArray<float, 2>{
                match[CENTER_DISTANCES_0].matched ? safe_stof(match[CENTER_DISTANCES_0].str()) : 0.f,
                match[CENTER_DISTANCES_1].matched ? safe_stof(match[CENTER_DISTANCES_1].str()) : float { INFINITY }},
            .cull_faces = safe_stob(match[CULL_FACES].str()),
            .emissivity = {
                match[EMISSIVITY_R].matched ? safe_stof(match[EMISSIVITY_R].str()) : 0.f,
                match[EMISSIVITY_G].matched ? safe_stof(match[EMISSIVITY_G].str()) : 0.f,
                match[EMISSIVITY_B].matched ? safe_stof(match[EMISSIVITY_B].str()) : 0.f},
            .ambience = {
                safe_stof(match[AMBIENCE_R].str()),
                safe_stof(match[AMBIENCE_G].str()),
                safe_stof(match[AMBIENCE_B].str())},
            .diffusivity = {
                safe_stof(match[DIFFUSIVITY_R].str()),
                safe_stof(match[DIFFUSIVITY_G].str()),
                safe_stof(match[DIFFUSIVITY_B].str())},
            .specularity = {
                safe_stof(match[SPECULARITY_R].str()),
                safe_stof(match[SPECULARITY_G].str()),
                safe_stof(match[SPECULARITY_B].str())}}.compute_color_mode()));
}
