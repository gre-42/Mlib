#pragma once
#include <string>

namespace Mlib {

enum class RoadConnectionType {
    AUTO,
    CENTRAL,
    ENDPOINT
};

RoadConnectionType road_connection_type_from_string(const std::string& s);

void road_connection_types_from_model_name(
    const std::string& model_name,
    RoadConnectionType& rct0,
    RoadConnectionType& rct1);

}
