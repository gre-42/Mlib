#include "Billboard_Atlas_Instance_Json.hpp"
#include <Mlib/Argument_List.hpp>
#include <Mlib/Geometry/Material/Billboard_Atlas_Instance.hpp>
#include <Mlib/Geometry/Material/Render_Pass.hpp>
#include <Mlib/Json/Json_View.hpp>

namespace BB {
BEGIN_ARGUMENT_LIST;
DECLARE_ARGUMENT(uv_scale);
DECLARE_ARGUMENT(uv_offset);
DECLARE_ARGUMENT(vertex_scale);
DECLARE_ARGUMENT(layer);
DECLARE_ARGUMENT(alpha_distances);
DECLARE_ARGUMENT(max_center_distance);
DECLARE_ARGUMENT(occluder_pass);
}
    
using namespace Mlib;

void Mlib::from_json(const nlohmann::json& j, BillboardAtlasInstance& bb) {
    JsonView jv{ j };
    jv.validate(BB::options);
    j.at(BB::uv_scale).get_to(bb.uv_scale);
    j.at(BB::uv_offset).get_to(bb.uv_offset);
    j.at(BB::vertex_scale).get_to(bb.vertex_scale);
    bb.texture_layer = jv.at<uint8_t>(BB::layer, 0);
    j.at(BB::alpha_distances).get_to(bb.alpha_distances);
    bb.max_center_distance2 = squared(json_get<ScenePos>(j.at(BB::max_center_distance)));
    bb.occluder_pass = external_render_pass_type_from_string(j.at(BB::occluder_pass).get<std::string>());
}
