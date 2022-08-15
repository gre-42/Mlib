#include "Create_Blending_X_Resource.hpp"
#include <Mlib/FPath.hpp>
#include <Mlib/Geometry/Material.hpp>
#include <Mlib/Regex_Select.hpp>
#include <Mlib/Render/Resources/Blending_X_Resource.hpp>
#include <Mlib/Scene/User_Function_Args.hpp>
#include <Mlib/Scene_Graph/Scene_Node_Resources.hpp>
#include <Mlib/Strings/From_Number.hpp>

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
DECLARE_OPTION(AGGREGATE_MODE);
DECLARE_OPTION(NUMBER_OF_FRAMES);

LoadSceneUserFunction CreateBlendingXResource::user_function = [](const LoadSceneUserFunctionArgs& args)
{
    static DECLARE_REGEX(regex,
        "^\\s*blending_x_resource"
        "\\s+name=([\\w+-.]+)"
        "\\s+texture_filename=(#?[\\w+-.\\(\\)/]+)"
        "\\s+min=([\\w+-.]+)\\s+([\\w+-.]+)"
        "\\s+max=([\\w+-.]+)\\s+([\\w+-.]+)"
        "\\s+aggregate_mode=(\\w+)"
        "(?:\\s+number_of_frames=(\\d+))?$");
    std::smatch match;
    if (Mlib::re::regex_match(args.line, match, regex)) {
        execute(match, args);
        return true;
    } else {
        return false;
    }
};

void CreateBlendingXResource::execute(
    const Mlib::re::smatch& match,
    const LoadSceneUserFunctionArgs& args)
{
    Material material{
        .blend_mode = BlendMode::CONTINUOUS,
        .textures = {{.texture_descriptor = {.color = args.fpath(match[TEXTURE_FILENAME].str()).path}}},
        .occluder_pass = ExternalRenderPassType::NONE,
        .wrap_mode_s = WrapMode::CLAMP_TO_EDGE,
        .wrap_mode_t = WrapMode::CLAMP_TO_EDGE,
        .aggregate_mode = aggregate_mode_from_string(match[AGGREGATE_MODE].str()),
        .number_of_frames = match[NUMBER_OF_FRAMES].matched ? safe_stoz(match[NUMBER_OF_FRAMES].str()) : 1,
        .cull_faces = false,
        .ambience = {2.f, 2.f, 2.f},
        .diffusivity = {0.f, 0.f, 0.f},
        .specularity = {0.f, 0.f, 0.f}};
    material.compute_color_mode();
    args.scene_node_resources.add_resource(match[NAME].str(), std::make_shared<BlendingXResource>(
        FixedArray<float, 2, 2>{
            safe_stof(match[MIN_X].str()), safe_stof(match[MIN_Y].str()),
            safe_stof(match[MAX_X].str()), safe_stof(match[MAX_Y].str())},
        material,
        material));
}
