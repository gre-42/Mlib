
#include "Macro_Recorder.hpp"
#include <Mlib/Macro_Executor/Macro_Line_Executor.hpp>
#include <Mlib/Macro_Executor/Replacement_Parameter.hpp>
#include <Mlib/Misc/Argument_List.hpp>
#include <Mlib/Os/Os.hpp>
#include <fstream>
#include <iostream>
#include <stdexcept>

using namespace Mlib;

MacroRecorder::MacroRecorder() = default;

MacroRecorder::~MacroRecorder() = default;

void MacroRecorder::operator()(const MacroLineExecutor& macro_line_executor)
{
    if (macro_line_executor.script_filename_.string().ends_with(".scn.json")) {
        std::scoped_lock lock{ include_mutex_ };
        if (included_files_.contains(macro_line_executor.script_filename_)) {
            return;
        }
        if (!included_files_.insert(macro_line_executor.script_filename_).second) {
            verbose_abort("Internal error, could not insert included file");
        }
        auto ifs = create_ifstream(macro_line_executor.script_filename_);
        if (ifs->fail()) {
            throw std::runtime_error("Could not open script file \"" + macro_line_executor.script_filename_.string() + '"');
        }
        if (macro_line_executor.verbose_) {
            linfo() << "Processing JSON scene file \"" << macro_line_executor.script_filename_ << '"';
        }
        nlohmann::json j;
        try {
            *ifs >> j;
        } catch (const nlohmann::json::exception& e) {
            throw std::runtime_error("Error loading file \"" + macro_line_executor.script_filename_.string() + "\": " + e.what());
        }
        if (!ifs->eof() && ifs->fail()) {
            throw std::runtime_error("Error reading from file: \"" + macro_line_executor.script_filename_.string() + '"');
        }
        macro_line_executor(j, nullptr);
    } else if (macro_line_executor.script_filename_.extension() == ".json") {
        auto rp = ReplacementParameterAndFilename::from_json(macro_line_executor.script_filename_);
        if (macro_line_executor.verbose_) {
            linfo() << "Processing JSON macro \"" << rp.rp.on_execute << '"';
        }
        macro_line_executor(rp.rp.on_execute, nullptr);
    } else {
        throw std::runtime_error("Unknown script file extension: \"" + macro_line_executor.script_filename_.string() + '"');
    }
}
