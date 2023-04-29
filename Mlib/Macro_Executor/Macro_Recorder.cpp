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
        macro_line_executor(e, &manifest.json_variables, nullptr);
    }
}
