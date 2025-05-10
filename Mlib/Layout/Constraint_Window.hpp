#include <nlohmann/json_fwd.hpp>
#include <string>

namespace Mlib {

struct ConstraintWindow {
    std::string left;
    std::string right;
    std::string bottom;
    std::string top;
};

void from_json(const nlohmann::json& j, ConstraintWindow& w);

}
