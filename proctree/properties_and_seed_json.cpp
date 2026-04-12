#include "properties_and_seed_json.hpp"
#include <Mlib/Json/Json_View.hpp>
#include <Mlib/Misc/Argument_List.hpp>
#include <Mlib/Strings/Str.hpp>
#include <proctree/proctree_json.hpp>
#include <proctree/properties_and_seed.hpp>

namespace KnownArgs {
BEGIN_ARGUMENT_LIST;
DECLARE_ARGUMENT(trunk_diffuse);
DECLARE_ARGUMENT(twig_diffuse);
DECLARE_ARGUMENT(properties);
DECLARE_ARGUMENT(seed);
}

using namespace Mlib;

void Proctree::from_json(const nlohmann::json& j, Proctree::PropertiesAndSeed& ps) {
    JsonView jv{j};
    jv.validate(KnownArgs::options);
    ps.trunk_diffuse = U8::u8str(jv.at<std::string>(KnownArgs::trunk_diffuse));
    ps.twig_diffuse = U8::u8str(jv.at<std::string>(KnownArgs::twig_diffuse));
    ps.properties = jv.at<Proctree::Properties>(KnownArgs::properties);
    ps.seed = jv.at<unsigned int>(KnownArgs::seed);
}
