#include "Macro_Recorder.hpp"
#include <Mlib/Argument_List.hpp>
#include <Mlib/Macro_Executor/Macro_Line_Executor.hpp>
#include <Mlib/Macro_Executor/Replacement_Parameter.hpp>
#include <Mlib/Os/Os.hpp>
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
        std::scoped_lock lock{ include_mutex_ };
        if (included_files_.contains(macro_line_executor.script_filename_)) {
            return;
        }
        if (!included_files_.insert(macro_line_executor.script_filename_).second) {
            verbose_abort("Internal error, could not insert included file");
        }
        auto ifs = create_ifstream(macro_line_executor.script_filename_);
        if (ifs->fail()) {
            THROW_OR_ABORT("Could not open script file \"" + macro_line_executor.script_filename_ + '"');
        }
        if (macro_line_executor.verbose_) {
            linfo() << "Processing JSON scene file \"" << macro_line_executor.script_filename_ << '"';
        }
        nlohmann::json j;
        try {
            *ifs >> j;
        } catch (const nlohmann::json::exception& e) {
            throw std::runtime_error("Error loading file \"" + macro_line_executor.script_filename_ + "\": " + e.what());
        }
        if (!ifs->eof() && ifs->fail()) {
            THROW_OR_ABORT("Error reading from file: \"" + macro_line_executor.script_filename_ + '"');
        }
        macro_line_executor(j, caller_args, nullptr);
    } else if (macro_line_executor.script_filename_.ends_with(".json")) {
        auto rp = ReplacementParameterAndFilename::from_json(macro_line_executor.script_filename_);
        if (macro_line_executor.verbose_) {
            linfo() << "Processing JSON macro \"" << rp.rp.on_execute << '"';
        }
        macro_line_executor(rp.rp.on_execute, nullptr, nullptr);
    } else {
        THROW_OR_ABORT("Unknown script file extension: \"" + macro_line_executor.script_filename_ + '"');
    }
}
