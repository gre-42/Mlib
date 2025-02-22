#include "Constraint_Window.hpp"
#include <Mlib/Argument_List.hpp>
#include <Mlib/Json/Misc.hpp>
#include <string>

namespace Mlib {

namespace KnownArgs {
BEGIN_ARGUMENT_LIST;
DECLARE_ARGUMENT(left);
DECLARE_ARGUMENT(right);
DECLARE_ARGUMENT(bottom);
DECLARE_ARGUMENT(top);
}

void from_json(const nlohmann::json& j, ConstraintWindow& w) {
    validate(j, KnownArgs::options);
    j.at(KnownArgs::left).get_to(w.left);
    j.at(KnownArgs::right).get_to(w.right);
    j.at(KnownArgs::bottom).get_to(w.bottom);
    j.at(KnownArgs::top).get_to(w.top);
}

}
