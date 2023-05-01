#include "Replacement_Parameter.hpp"
#include <Mlib/Argument_List.hpp>

namespace fs = std::filesystem;

using namespace Mlib;

namespace KnownArgs {
BEGIN_ARGUMENT_LIST;
DECLARE_ARGUMENT(title);
DECLARE_ARGUMENT(variables);
DECLARE_ARGUMENT(required);
}

void Mlib::from_json(const nlohmann::json& j, ReplacementParameter& rp) {
    validate(j, KnownArgs::options);
    j.at(KnownArgs::title).get_to(rp.title);
    if (j.contains(KnownArgs::variables)) {
        rp.variables.merge(JsonMacroArguments{j.at(KnownArgs::variables)});
    }
    if (j.contains(KnownArgs::required)) {
        j.at(KnownArgs::required).get_to(rp.required);
    }
}
