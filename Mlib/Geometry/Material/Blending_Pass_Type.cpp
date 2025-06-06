#include "Blending_Pass_Type.hpp"
#include <Mlib/Throw_Or_Abort.hpp>
#include <map>

using namespace Mlib;

BlendingPassType Mlib::blending_pass_type_from_string(const std::string& str) {
    static const std::map<std::string, BlendingPassType> m{
        {"none", BlendingPassType::NONE},
        {"early", BlendingPassType::EARLY},
        {"late", BlendingPassType::LATE}
    };
    auto it = m.find(str);
    if (it == m.end()) {
        THROW_OR_ABORT("Unknown blending pass type: \"" + str + '"');
    }
    return it->second;
}
