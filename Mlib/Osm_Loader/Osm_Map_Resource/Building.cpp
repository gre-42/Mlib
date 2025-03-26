#include "Building.hpp"
#include <Mlib/Argument_List.hpp>
#include <Mlib/Json/Json_View.hpp>

using namespace Mlib;

namespace Roof9_2Args {
BEGIN_ARGUMENT_LIST;
DECLARE_ARGUMENT(width);
DECLARE_ARGUMENT(height);
}

void Mlib::from_json(const nlohmann::json& j, Roof9_2& roof9_2) {
    JsonView jv{j};
    jv.validate(Roof9_2Args::options);
    jv.at(Roof9_2Args::width).get_to(roof9_2.width);
    jv.at(Roof9_2Args::height).get_to(roof9_2.height);
}
