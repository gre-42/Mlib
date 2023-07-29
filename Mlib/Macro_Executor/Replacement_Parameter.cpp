#include "Replacement_Parameter.hpp"
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
DECLARE_ARGUMENT(on_execute);
DECLARE_ARGUMENT(title);
DECLARE_ARGUMENT(globals);
DECLARE_ARGUMENT(database);
DECLARE_ARGUMENT(required);
}

ReplacementParameterAndFilename ReplacementParameterAndFilename::from_json(const std::string& filename) {
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
        return ReplacementParameterAndFilename{
            .rp = j.get<ReplacementParameter>(),
            .filename = filename};
    } catch (const nlohmann::json::exception& e) {
        throw std::runtime_error("Error loading file \"" + filename + "\": " + e.what());
    }
}

void Mlib::from_json(const nlohmann::json& j, ReplacementParameter& rp) {
    validate(j, KnownArgs::options);
    j.at(KnownArgs::title).get_to(rp.title);
    if (j.contains(KnownArgs::globals)) {
        rp.globals.merge(JsonMacroArguments{j.at(KnownArgs::globals)});
    }
    if (j.contains(KnownArgs::database)) {
        rp.database.merge(JsonMacroArguments{j.at(KnownArgs::database)});
    }
    if (j.contains(KnownArgs::required)) {
        j.at(KnownArgs::required).get_to(rp.required);
    }
    j.at(KnownArgs::id).get_to(rp.id);
    if (j.contains(KnownArgs::on_init)) {
        j.at(KnownArgs::on_init).get_to(rp.on_init);
    }
    if (j.contains(KnownArgs::on_execute)) {
        j.at(KnownArgs::on_execute).get_to(rp.on_execute);
    }
}
