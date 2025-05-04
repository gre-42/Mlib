#include "Parse_Position.hpp"
#include <Mlib/Json/Misc.hpp>
#include <Mlib/Math/Transformation/Transformation_Matrix.hpp>
#include <Mlib/Regex/Regex_Select.hpp>
#include <Mlib/Scene_Graph/Resources/Scene_Node_Resources.hpp>

using namespace Mlib;

static FixedArray<ScenePos, 3> parse_geographic_position(
    const TransformationMatrix<double, double, 3>* inverse_geographic_coordinates,
    const std::string& x_str,
    const std::string& y_str,
    const std::string& z_str)
{
    static const DECLARE_REGEX(re, "^([\\deE.+-]+)_deg");
    Mlib::re::cmatch match_x;
    Mlib::re::cmatch match_y;
    bool mx = Mlib::re::regex_match(x_str, match_x, re);
    bool my = Mlib::re::regex_match(y_str, match_y, re);
    if (mx != my) {
        THROW_OR_ABORT("Inconsistent positions: " + x_str + ", " + y_str);
    }
    if (mx) {
        if (inverse_geographic_coordinates == nullptr) {
            THROW_OR_ABORT("World coordinates not defined");
        }
        return inverse_geographic_coordinates->transform(
            FixedArray<double, 3>{
                safe_stod(match_y[1].str()),
                safe_stod(match_x[1].str()),
                safe_stod(z_str)}).casted<ScenePos>();
    } else {
        return FixedArray<ScenePos, 3>{
            safe_stof(x_str),
            safe_stof(y_str),
            safe_stof(z_str)};
    }
}

FixedArray<CompressedScenePos, 3> Mlib::parse_position(
    const nlohmann::json& j,
    SceneNodeResources& scene_node_resources)
{
    FixedArray<ScenePos, 3> pos = uninitialized;
    // root nodes do not have a default pose
    auto jpos = j.get<UFixedArray<nlohmann::json, 3>>();
    if ((jpos(0).type() == nlohmann::detail::value_t::string) &&
        (jpos(1).type() == nlohmann::detail::value_t::string) &&
        (jpos(2).type() == nlohmann::detail::value_t::string))
    {
        pos = parse_geographic_position(
            scene_node_resources.get_geographic_mapping(VariableAndHash<std::string>{"world.inverse"}),
            jpos(0).get<std::string>(),
            jpos(1).get<std::string>(),
            jpos(2).get<std::string>());
    } else {
        pos = jpos.applied<ScenePos>([](const nlohmann::json& j){return j.get<ScenePos>();});
    }
    return pos.casted<CompressedScenePos>();
}
