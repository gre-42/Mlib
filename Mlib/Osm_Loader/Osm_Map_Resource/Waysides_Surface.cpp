#include "Waysides_Surface.hpp"
#include <Mlib/Argument_List.hpp>
#include <Mlib/Json/Json_View.hpp>
#include <Mlib/Physics/Units.hpp>
#include <Mlib/Render/Rendering_Context.hpp>
#include <Mlib/Scene_Graph/Resources/Parsed_Resource_Name.hpp>

using namespace Mlib;

namespace Fields {
BEGIN_ARGUMENT_LIST;
DECLARE_ARGUMENT(resources);
DECLARE_ARGUMENT(tangential_distance);
DECLARE_ARGUMENT(normal_distance);
DECLARE_ARGUMENT(gradient_dx);
DECLARE_ARGUMENT(max_gradient);
}

void Mlib::from_json(const nlohmann::json& j, WaysideResourceNamesSurface& w) {
    JsonView jv{ j };
    jv.validate(Fields::options, "surface wayside: ");
    w.tangential_distance = jv.at<float>(Fields::tangential_distance) * meters;
    w.normal_distance = jv.at<float>(Fields::normal_distance) * meters;
    w.gradient_dx = jv.at<float>(Fields::gradient_dx) * meters;
    w.max_gradient = jv.at<float>(Fields::max_gradient) * meters;
    auto& scene_node_resources = RenderingContextStack::primary_scene_node_resources();
    auto parse_resource_name_func = [&scene_node_resources](const std::string& jma){
        return parse_resource_name(scene_node_resources, jma);
    };
    w.resource_names = jv.at_vector<std::string>(Fields::resources, parse_resource_name_func);
}
