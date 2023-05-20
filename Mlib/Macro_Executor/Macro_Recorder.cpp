#include "Macro_Recorder.hpp"
#include <Mlib/Argument_List.hpp>
#include <Mlib/Macro_Executor/Macro_Line_Executor.hpp>
#include <Mlib/Macro_Executor/Macro_Manifest.hpp>
#include <Mlib/Os/Os.hpp>
#include <Mlib/Regex_Select.hpp>
#include <Mlib/Throw_Or_Abort.hpp>
#include <fstream>
#include <iostream>

using namespace Mlib;

MacroRecorder::MacroRecorder() = default;

MacroRecorder::~MacroRecorder() = default;

void MacroRecorder::operator()(
    const MacroLineExecutor& macro_line_executor,
    const JsonMacroArguments* caller_args)
{
    if (macro_line_executor.script_filename_.ends_with(".scn.json")) {
        auto ifs_p = create_ifstream(macro_line_executor.script_filename_);
        auto& ifs = *ifs_p;
        if (ifs.fail()) {
            THROW_OR_ABORT("Could not open script file \"" + macro_line_executor.script_filename_ + '"');
        }
        if (macro_line_executor.verbose_) {
            linfo() << "Processing JSON scene file \"" << macro_line_executor.script_filename_ << '"';
        }
        nlohmann::json j;
        try {
            ifs >> j;
        } catch (const nlohmann::json::exception& e) {
            throw std::runtime_error("Error loading file \"" + macro_line_executor.script_filename_ + "\": " + e.what());
        }
        if (!ifs.eof() && ifs.fail()) {
            THROW_OR_ABORT("Error reading from file: \"" + macro_line_executor.script_filename_ + '"');
        }
        macro_line_executor(JsonView{j}, caller_args, nullptr);
    } else if (macro_line_executor.script_filename_.ends_with(".json")) {
        auto manifest = MacroManifest::load_from_json(macro_line_executor.script_filename_);
        if (macro_line_executor.verbose_) {
            linfo() << "Processing JSON macro \"" << manifest.macro << '"';
        }
        macro_line_executor(JsonView{manifest.macro}, nullptr, nullptr);
    } else {
        THROW_OR_ABORT("Unknown script file extension: \"" + macro_line_executor.script_filename_ + '"');
    }
}
