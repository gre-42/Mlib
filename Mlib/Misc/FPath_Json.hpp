#pragma once
#include <nlohmann/json_fwd.hpp>

namespace Mlib {

class FPath;

void from_json(const nlohmann::json& j, FPath& p);

}
