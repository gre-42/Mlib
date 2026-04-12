#include "FPath_Json.hpp"
#include <Mlib/Json/Base.hpp>
#include <Mlib/Misc/FPath.hpp>
#include <Mlib/Strings/Str.hpp>

using namespace Mlib;

void Mlib::from_json(const nlohmann::json& j, FPath& p) {
    p = FPath{U8::u8str(j.get<std::string>())};
}
