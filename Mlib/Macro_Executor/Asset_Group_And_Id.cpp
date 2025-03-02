#include "Asset_Group_And_Id.hpp"
#include <Mlib/Argument_List.hpp>
#include <Mlib/Json/Misc.hpp>

using namespace Mlib;

namespace KnownArgs {
BEGIN_ARGUMENT_LIST;
DECLARE_ARGUMENT(group);
DECLARE_ARGUMENT(id);
}
    
void Mlib::from_json(const nlohmann::json& j, AssetGroupAndId& gid) {
    validate(j, KnownArgs::options);
    gid.group = VariableAndHash{ j.at(KnownArgs::group).get<std::string>() };
    gid.id = VariableAndHash{ j.at(KnownArgs::id).get<std::string>() };
}
