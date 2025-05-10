#include "Blend_Map_Texture_Json.hpp"
#include <Mlib/Argument_List.hpp>
#include <Mlib/Geometry/Material/Blend_Map_Texture.hpp>
#include <Mlib/Macro_Executor/Json_Macro_Arguments.hpp>
#include <Mlib/Render/Rendering_Context.hpp>
#include <Mlib/Render/Resource_Managers/Rendering_Resources.hpp>

using namespace Mlib;

namespace KnownArgs {
BEGIN_ARGUMENT_LIST;
DECLARE_ARGUMENT(texture);
DECLARE_ARGUMENT(min_height);
DECLARE_ARGUMENT(max_height);
DECLARE_ARGUMENT(distances);
DECLARE_ARGUMENT(normal);
DECLARE_ARGUMENT(cosine);
DECLARE_ARGUMENT(discreteness);
DECLARE_ARGUMENT(scale);
DECLARE_ARGUMENT(weight);
DECLARE_ARGUMENT(plus);
DECLARE_ARGUMENT(min_detail_weight);
DECLARE_ARGUMENT(role);
DECLARE_ARGUMENT(uv_source);
DECLARE_ARGUMENT(reduction);
DECLARE_ARGUMENT(reweight);
}

BlendMapTexture Mlib::blend_map_texture_from_json(const JsonMacroArguments& j)
{
    j.validate(KnownArgs::options);
    auto& rr = RenderingContextStack::primary_rendering_resources();
    auto tex = j.path_or_variable(KnownArgs::texture);
    return {
        .texture_descriptor = tex.is_variable
            ? rr.get_texture_descriptor(VariableAndHash{tex.path})
            : TextureDescriptor{ .color = {.filename = VariableAndHash{tex.path}} },
        .min_height = j.at<float>(KnownArgs::min_height),
        .max_height = j.at<float>(KnownArgs::max_height),
        .distances = j.at<UOrderableFixedArray<float, 4>>(KnownArgs::distances),
        .normal = j.at<UOrderableFixedArray<float, 3>>(KnownArgs::normal),
        .cosines = j.at<UOrderableFixedArray<float, 4>>(KnownArgs::cosine),
        .discreteness = j.at<float>(KnownArgs::discreteness, 2.f),
        .scale = j.at<UOrderableFixedArray<float, 2>>(KnownArgs::scale),
        .weight = j.at<float>(KnownArgs::weight),
        .plus = j.at<float>(KnownArgs::plus, 0.f),
        .min_detail_weight = j.at<float>(KnownArgs::min_detail_weight, 0.f),
        .role = blend_map_role_from_string(j.at<std::string>(KnownArgs::role, "summand")),
        .uv_source = blend_map_uv_source_from_string(j.at<std::string>(KnownArgs::uv_source, "vertical0")),
        .reduction = blend_map_reduction_operation_from_string(j.at<std::string>(KnownArgs::reduction, "plus")),
        .reweight_mode = blend_map_reweight_mode_from_string(j.at<std::string>(KnownArgs::reweight, "undefined"))
    };
}

std::vector<BlendMapTexture> Mlib::blend_map_textures_from_json(
    const JsonMacroArguments& j,
    std::string_view name)
{
    auto& rr = RenderingContextStack::primary_rendering_resources();
    return j.at_vector_non_null_optional<nlohmann::json>(name, [&](const auto& c){
        if (c.type() == nlohmann::detail::value_t::string) {
            return rr.get_blend_map_texture(c.template get<VariableAndHash<std::string>>());
        } else {
            return blend_map_texture_from_json(j.as_child(c));
        }
    });
}

std::vector<BlendMapTexture> Mlib::try_blend_map_textures_from_json(
    const JsonMacroArguments& j,
    std::string_view name)
{
    if (!j.contains(name)) {
        return {};
    }
    return blend_map_textures_from_json(j, name);
}
