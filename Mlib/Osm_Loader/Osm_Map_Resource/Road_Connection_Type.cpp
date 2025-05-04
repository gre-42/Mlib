#include "Road_Connection_Type.hpp"
#include <Mlib/Regex/Regex_Select.hpp>
#include <Mlib/Throw_Or_Abort.hpp>
#include <Mlib/Variable_And_Hash.hpp>

using namespace Mlib;

static const auto CENTRAL = VariableAndHash<std::string>{ "central" };
static const auto ENDPOINT = VariableAndHash<std::string>{ "endpoint" };

RoadConnectionType Mlib::road_connection_type_from_string(const VariableAndHash<std::string>& s) {
    if (s == CENTRAL) {
        return RoadConnectionType::CENTRAL;
    } else if (s == ENDPOINT) {
        return RoadConnectionType::ENDPOINT;
    } else {
        THROW_OR_ABORT("Unknown road connection type: \"" + *s + '"');
    }
}

void Mlib::road_connection_types_from_model_name(
    const VariableAndHash<std::string>& model_name,
    RoadConnectionType& rct0,
    RoadConnectionType& rct1)
{
    if (model_name->empty()) {
        rct0 = RoadConnectionType::AUTO;
        rct1 = RoadConnectionType::AUTO;
    } else if (model_name == ENDPOINT) {
        rct0 = RoadConnectionType::ENDPOINT;
        rct1 = RoadConnectionType::ENDPOINT;
    } else {
        static const DECLARE_REGEX(re, "^[^.]+\\.(\\w+)-(\\w+)$");
        Mlib::re::cmatch match;
        if (!Mlib::re::regex_match(*model_name, match, re)) {
            THROW_OR_ABORT("Could not parse model name: \"" + *model_name + '"');
        }
        rct0 = road_connection_type_from_string(VariableAndHash<std::string>{match[1].str()});
        rct1 = road_connection_type_from_string(VariableAndHash<std::string>{match[2].str()});
    }
}
