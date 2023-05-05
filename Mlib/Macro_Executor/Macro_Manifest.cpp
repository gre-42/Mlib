#include "Macro_Manifest.hpp"
#include <Mlib/Argument_List.hpp>
#include <Mlib/Json/Misc.hpp>
#include <Mlib/Macro_Executor/Macro_Keys.hpp>
#include <Mlib/Os/Os.hpp>
#include <Mlib/Throw_Or_Abort.hpp>
#include <filesystem>
#include <fstream>

namespace fs = std::filesystem;

using namespace Mlib;

namespace KnownArgs {
BEGIN_ARGUMENT_LIST;
DECLARE_ARGUMENT(required);
DECLARE_ARGUMENT(macro);
DECLARE_ARGUMENT(variables);
DECLARE_ARGUMENT(name);
}

MacroManifest MacroManifest::from_json(const std::string& filename) {
    MacroManifest result;
    try {
        nlohmann::json j;
        auto ifs_p = create_ifstream(filename);
        auto& ifs = *ifs_p;
        if (ifs.fail()) {
            THROW_OR_ABORT("Could not open macro manifest file \"" + filename + '"');
        }
        ifs >> j;
        if (!ifs.eof() && ifs.fail()) {
            THROW_OR_ABORT("Error reading from file: \"" + filename + '"');
        }
        validate(j, KnownArgs::options);
        result.variables.insert_json(j.at(KnownArgs::variables));
        result.requires_ = j.at(KnownArgs::required).get<std::vector<std::string>>();
        result.macro = j.at(KnownArgs::macro);
        result.name = j.at(KnownArgs::name);
    } catch (const nlohmann::json::exception& e) {
        throw std::runtime_error("Error loading file \"" + filename + "\": " + e.what());
    }
    return result;
}
