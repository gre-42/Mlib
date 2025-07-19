#pragma once
#include <nlohmann/json_fwd.hpp>
#include <string>
#include <vector>

namespace Mlib {

using BooleanExpression = std::vector<std::vector<std::string>>;

void expression_from_json(const nlohmann::json& j, std::vector<std::vector<std::string>>& expression);

}
