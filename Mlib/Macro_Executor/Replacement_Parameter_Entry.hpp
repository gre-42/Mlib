#pragma once
#include <Mlib/Macro_Executor/Replacement_Parameter.hpp>

namespace Mlib {

struct ReplacementParameterEntry {
    static ReplacementParameterEntry from_json(const std::string& filename);
    std::string id;
    nlohmann::json on_init;
    ReplacementParameter params;
};

void from_json(const nlohmann::json& j, ReplacementParameterEntry& rp);

}
