#include "Color_And_Probability.hpp"
#include <Mlib/Argument_List.hpp>
#include <Mlib/Json/Json_View.hpp>

using namespace Mlib;

namespace KnownAttributes {
BEGIN_ARGUMENT_LIST;
DECLARE_ARGUMENT(color);
DECLARE_ARGUMENT(probability);
}

void Mlib::from_json(const nlohmann::json& j, ColorAndProbability& cp) {
    JsonView jv{ j };
    jv.validate(KnownAttributes::options);
    j.at(KnownAttributes::color).get_to(cp.color);
    j.at(KnownAttributes::probability).get_to(cp.probability);
}
