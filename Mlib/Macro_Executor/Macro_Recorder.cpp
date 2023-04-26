#include "Macro_Recorder.hpp"
#include <Mlib/Argument_List.hpp>
#include <Mlib/Macro_Executor/Macro_Line_Executor.hpp>
#include <Mlib/Macro_Executor/Macro_Manifest.hpp>
#include <Mlib/Os/Os.hpp>
#include <Mlib/Regex_Select.hpp>
#include <Mlib/Throw_Or_Abort.hpp>
#include <fstream>
#include <iostream>

using json = nlohmann::json;
using namespace Mlib;

namespace KnownArgs {
BEGIN_ARGUMENT_LIST;
DECLARE_ARGUMENT(declare_macro);
DECLARE_ARGUMENT(content);
}

void MacroRecorder::operator()(const MacroLineExecutor& macro_line_executor)
{
    auto manifest = MacroManifest::from_json(macro_line_executor.script_filename_);
    if (!manifest.script_file.ends_with(".scn.json")) {
        THROW_OR_ABORT("Unknown script file extension: \"" + manifest.script_file + '"');
    }

    auto ifs_p = create_ifstream(manifest.script_file);
    auto& ifs = *ifs_p;
    if (ifs.fail()) {
        THROW_OR_ABORT("Could not open script file \"" + manifest.script_file + '"');
    }
    if (macro_line_executor.verbose_) {
        linfo() << "Processing JSON scene file \"" << manifest.script_file << '"';
    }
    json j;
    ifs >> j;
    if (!ifs.eof() && ifs.fail()) {
        THROW_OR_ABORT("Error reading from file: \"" + macro_line_executor.script_filename_ + '"');
    }
    if (j.type() != nlohmann::detail::value_t::array) {
        THROW_OR_ABORT("Unexpected element in file: \"" + macro_line_executor.script_filename_ + '"');
    }
    for (const auto& e : j) {
        if (e.contains(KnownArgs::declare_macro)) {
            validate(e, KnownArgs::options);
            auto name = e.at(KnownArgs::declare_macro).get<std::string>();
            if (macro_line_executor.verbose_) {
                linfo() << "Storing macro \"" << name << '"';
            }
            if (!json_macros_.try_emplace(
                name,
                JsonMacro{
                    .filename = manifest.script_file,
                    .content = e.at(KnownArgs::content)
                }).second)
            {
                THROW_OR_ABORT("Macro with name \"" + name + "\" already exists");
            }
        } else {
            macro_line_executor(e, &manifest.json_variables, nullptr);
        }
    }
}
