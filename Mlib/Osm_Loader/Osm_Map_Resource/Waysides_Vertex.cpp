#include "Waysides_Vertex.hpp"
#include <Mlib/Argument_List.hpp>
#include <Mlib/Json/Json_View.hpp>
#include <Mlib/Render/Rendering_Context.hpp>
#include <Mlib/Scene_Graph/Resources/Parsed_Resource_Name.hpp>

using namespace Mlib;

namespace Fields {
BEGIN_ARGUMENT_LIST;
DECLARE_ARGUMENT(resources);
DECLARE_ARGUMENT(min_dist);
DECLARE_ARGUMENT(max_dist);
}

void Mlib::from_json(const nlohmann::json& j, WaysideResourceNamesVertex& w) {
    JsonView jv{ j };
    jv.validate(Fields::options, "vertex wayside: ");
    jv.at(Fields::min_dist).get_to(w.min_dist);
    jv.at(Fields::max_dist).get_to(w.max_dist);
    auto& scene_node_resources = RenderingContextStack::primary_scene_node_resources();
    auto parse_resource_name_func = [&scene_node_resources](const std::string& jma){
        return parse_resource_name(scene_node_resources, jma);
    };
    w.resource_names = jv.at_vector<std::string>(Fields::resources, parse_resource_name_func);
}
