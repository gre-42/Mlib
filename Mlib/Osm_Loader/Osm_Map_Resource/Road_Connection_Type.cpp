#include "Road_Connection_Type.hpp"
#include <Mlib/Regex/Regex_Select.hpp>
#include <Mlib/Throw_Or_Abort.hpp>

using namespace Mlib;

RoadConnectionType Mlib::road_connection_type_from_string(const std::string& s) {
    if (s == "central") {
        return RoadConnectionType::CENTRAL;
    } else if (s == "endpoint") {
        return RoadConnectionType::ENDPOINT;
    } else {
        THROW_OR_ABORT("Unknown road connection type: \"" + s + '"');
    }
}

void Mlib::road_connection_types_from_model_name(
    const std::string& model_name,
    RoadConnectionType& rct0,
    RoadConnectionType& rct1)
{
    if (model_name.empty()) {
        rct0 = RoadConnectionType::AUTO;
        rct1 = RoadConnectionType::AUTO;
    } else if (model_name == "endpoint") {
        rct0 = RoadConnectionType::ENDPOINT;
        rct1 = RoadConnectionType::ENDPOINT;
    } else {
        static const DECLARE_REGEX(re, "^[^.]+\\.(\\w+)-(\\w+)$");
        Mlib::re::smatch match;
        if (!Mlib::re::regex_match(model_name, match, re)) {
            THROW_OR_ABORT("Could not parse model name: \"" + model_name + '"');
        }
        rct0 = road_connection_type_from_string(match[1].str());
        rct1 = road_connection_type_from_string(match[2].str());
    }
}
