#pragma once
#include <string>

namespace Mlib {

template <class T>
class VariableAndHash;
enum class RoadConnectionType {
    AUTO,
    CENTRAL,
    ENDPOINT
};

RoadConnectionType road_connection_type_from_string(const VariableAndHash<std::string>& s);

void road_connection_types_from_model_name(
    const VariableAndHash<std::string>& model_name,
    RoadConnectionType& rct0,
    RoadConnectionType& rct1);

}
