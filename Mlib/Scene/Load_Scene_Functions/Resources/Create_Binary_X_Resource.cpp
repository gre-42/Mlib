#include "Create_Binary_X_Resource.hpp"
#include <Mlib/FPath.hpp>
#include <Mlib/Geometry/Material.hpp>
#include <Mlib/Regex_Select.hpp>
#include <Mlib/Render/Resources/Binary_X_Resource.hpp>
#include <Mlib/Scene/User_Function_Args.hpp>
#include <Mlib/Scene_Graph/Scene_Node_Resources.hpp>

using namespace Mlib;

#define BEGIN_OPTIONS static size_t option_id = 1
#define DECLARE_OPTION(a) static const size_t a = option_id++

BEGIN_OPTIONS;
DECLARE_OPTION(NAME);
DECLARE_OPTION(TEXTURE_FILENAME_0);
DECLARE_OPTION(TEXTURE_FILENAME_90);
DECLARE_OPTION(MIN_X);
DECLARE_OPTION(MIN_Y);
DECLARE_OPTION(MAX_X);
DECLARE_OPTION(MAX_Y);
DECLARE_OPTION(CENTER_DISTANCES_0);
DECLARE_OPTION(CENTER_DISTANCES_1);
DECLARE_OPTION(OCCLUDED_PASS);
DECLARE_OPTION(OCCLUDER_PASS);
DECLARE_OPTION(AMBIENCE_R);
DECLARE_OPTION(AMBIENCE_G);
DECLARE_OPTION(AMBIENCE_B);
DECLARE_OPTION(BLEND_MODE);
DECLARE_OPTION(ALPHA_DISTANCES_0);
DECLARE_OPTION(ALPHA_DISTANCES_1);
DECLARE_OPTION(ALPHA_DISTANCES_2);
DECLARE_OPTION(ALPHA_DISTANCES_3);
DECLARE_OPTION(CULL_FACES);
DECLARE_OPTION(AGGREGATE_MODE);
DECLARE_OPTION(TRANSFORMATION_MODE);

LoadSceneUserFunction CreateBinaryXResource::user_function = [](const LoadSceneUserFunctionArgs& args)
{
    static DECLARE_REGEX(regex,
        "^\\s*binary_x_resource"
        "\\s+name=([\\w+-.]+)"
        "\\s+texture_filename_0=(#?[\\w+-.\\(\\)/]+)"
        "\\s+texture_filename_90=(#?[\\w+-.\\(\\)/]+)"
        "\\s+min=([\\w+-.]+)\\s+([\\w+-.]+)"
        "\\s+max=([\\w+-.]+)\\s+([\\w+-.]+)"
        "(?:\\s+center_distances=([\\w+-.]+)\\s+([\\w+-.]+))?"
        "\\s+occluded_pass=(\\w+)"
        "\\s+occluder_pass=(\\w+)"
        "\\s+ambience=([\\w+-.]+)\\s+([\\w+-.]+)\\s+([\\w+-.]+)"
        "\\s+blend_mode=(\\w+)"
        "\\s+alpha_distances=([\\w+-.]+)\\s+([\\w+-.]+)\\s+([\\w+-.]+)\\s+([\\w+-.]+)"
        "\\s+cull_faces=(0|1)"
        "\\s+aggregate_mode=(\\w+)"
        "\\s+transformation_mode=(\\w+)$");
    std::smatch match;
    if (Mlib::re::regex_match(args.line, match, regex)) {
        execute(match, args);
        return true;
    } else {
        return false;
    }
};

void CreateBinaryXResource::execute(
    const Mlib::re::smatch& match,
    const LoadSceneUserFunctionArgs& args)
{
    FixedArray<float, 2, 2> square{
        safe_stof(match[MIN_X].str()), safe_stof(match[MIN_Y].str()),
        safe_stof(match[MAX_X].str()), safe_stof(match[MAX_Y].str())};
    Material material{
        .blend_mode = blend_mode_from_string(match[BLEND_MODE].str()),
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
        .center_distances = OrderableFixedArray<float, 2>{
            match[CENTER_DISTANCES_0].matched ? safe_stof(match[CENTER_DISTANCES_0].str()) : 0.f,
            match[CENTER_DISTANCES_1].matched ? safe_stof(match[CENTER_DISTANCES_1].str()) : float { INFINITY }},
        .cull_faces = safe_stob(match[CULL_FACES].str()),
        .ambience = {
            safe_stof(match[AMBIENCE_R].str()),
            safe_stof(match[AMBIENCE_G].str()),
            safe_stof(match[AMBIENCE_B].str())},
        .diffusivity = {0.f, 0.f, 0.f},
        .specularity = {0.f, 0.f, 0.f}};
    Material material_0{material};
    Material material_90{material};
    material_0.textures = {{.texture_descriptor = {.color = args.fpath(match[TEXTURE_FILENAME_0].str()).path}}};
    material_90.textures = {{.texture_descriptor = {.color = args.fpath(match[TEXTURE_FILENAME_90].str()).path}}};
    material_0.compute_color_mode();
    material_90.compute_color_mode();
    args.scene_node_resources.add_resource_loader(
        match[NAME].str(),
        [square, material_0, material_90](){return std::make_shared<BinaryXResource>(
            square,
            material_0,
            material_90);});
}
