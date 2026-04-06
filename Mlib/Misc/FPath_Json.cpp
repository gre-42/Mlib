#include "FPath_Json.hpp"
#include <Mlib/Json/Base.hpp>
#include <Mlib/Misc/FPath.hpp>

using namespace Mlib;

void Mlib::from_json(const nlohmann::json& j, FPath& p) {
    p = FPath{j.get<std::string>()};
}
