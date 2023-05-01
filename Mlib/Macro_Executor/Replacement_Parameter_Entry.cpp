#include "Replacement_Parameter_Entry.hpp"
#include <Mlib/Argument_List.hpp>
#include <Mlib/Json/Misc.hpp>
#include <Mlib/Os/Os.hpp>
#include <Mlib/Throw_Or_Abort.hpp>
#include <filesystem>
#include <fstream>

namespace fs = std::filesystem;

using namespace Mlib;

namespace KnownArgs {
BEGIN_ARGUMENT_LIST;
DECLARE_ARGUMENT(id);
DECLARE_ARGUMENT(on_init);
DECLARE_ARGUMENT(params);
}

ReplacementParameterEntry ReplacementParameterEntry::from_json(const std::string& filename) {
    try {
        nlohmann::json j;
        auto ifs_p = create_ifstream(filename);
        auto& ifs = *ifs_p;
        if (ifs.fail()) {
            THROW_OR_ABORT("Could not open replacement parameter entry file \"" + filename + '"');
        }
        ifs >> j;
        if (!ifs.eof() && ifs.fail()) {
            THROW_OR_ABORT("Error reading from file: \"" + filename + '"');
        }
        return j.get<ReplacementParameterEntry>();
    } catch (const nlohmann::json::exception& e) {
        throw std::runtime_error("Error loading file \"" + filename + "\": " + e.what());
    }
}

void Mlib::from_json(const nlohmann::json& j, ReplacementParameterEntry& rp) {
    validate(j, KnownArgs::options);
    j.at(KnownArgs::id).get_to(rp.id);
    if (j.contains(KnownArgs::on_init)) {
        j.at(KnownArgs::on_init).get_to(rp.on_init);
    }
    j.at(KnownArgs::params).get_to(rp.params);
}
