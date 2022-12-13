#include "Macro_Recorder.hpp"
#include <Mlib/Macro_Executor/Macro_Line_Executor.hpp>
#include <Mlib/Macro_Executor/Macro_Manifest.hpp>
#include <Mlib/Os/Os.hpp>
#include <Mlib/Regex_Select.hpp>
#include <fstream>
#include <iostream>

using namespace Mlib;

void MacroRecorder::operator()(const MacroLineExecutor& macro_line_executor, const RegexSubstitutionCache& rsc)
{
    MacroManifest manifest{macro_line_executor.script_filename_};
    auto ifs_p = create_ifstream(manifest.script_file);
    auto& ifs = *ifs_p;
    if (ifs.fail()) {
        throw std::runtime_error("Could not open script file \"" + manifest.script_file + '"');
    }

    static const DECLARE_REGEX(macro_begin_reg, "^\\s*macro_begin ([\\w+-.]+)$");
    static const DECLARE_REGEX(macro_end_reg, "^\\s*macro_end$");
    static const DECLARE_REGEX(alias_reg, "^\\s*global ([\\w+-.]+)\\s*=\\s*([\\w+-.]*)$");

    std::string line;
    std::list<std::pair<std::string, Macro>> recording_macros;
    while (std::getline(ifs, line, ';')) {
        if (line.length() > 0 && line[line.length() - 1] == '\r') {
            line = line.substr(0, line.length() - 1);
        }
        Mlib::re::smatch match;

        if (Mlib::re::regex_match(line, match, macro_begin_reg)) {
            // std::cerr << macro_line_executor.global_substitutions_ << std::endl;
            if (macro_line_executor.verbose_) {
                std::cerr << "Recording macro \"" << match[1].str() << '"' << std::endl;
            }
            recording_macros.push_back(std::make_pair(match[1].str(), Macro{.filename = manifest.script_file}));
        } else if (Mlib::re::regex_match(line, match, macro_end_reg)) {
            if (macro_line_executor.verbose_) {
                std::cerr << "Finishing macro \"" << recording_macros.back().first << '"' << std::endl;
            }
            if (recording_macros.empty()) {
                throw std::runtime_error("Macro-end despite no active macro");
            }
            auto it = macros_.insert(std::make_pair(recording_macros.back().first, recording_macros.back().second));
            if (!it.second) {
                throw std::runtime_error("Duplicate macro: " + recording_macros.back().first);
            }
            recording_macros.pop_back();
        } else if (!recording_macros.empty()) {
            if (macro_line_executor.verbose_) {
                std::cerr << "Adding line to macro \"" << recording_macros.back().first << '"' << std::endl;
            }
            recording_macros.back().second.lines.push_back(line);
        } else if (Mlib::re::regex_match(line, match, alias_reg)) {
            if (!globals_.insert(match[1].str(), match[2].str())) {
                throw std::runtime_error("Global variable \"" + match[1].str() + "\" already exists");
            }
        } else {
            macro_line_executor(line, &manifest.variables, rsc);
        }
    }

    if (!ifs.eof() && ifs.fail()) {
        throw std::runtime_error("Error reading from file: \"" + macro_line_executor.script_filename_ + '"');
    }

    if (!recording_macros.empty()) {
        throw std::runtime_error("Missing macro_end; in file \"" + macro_line_executor.script_filename_ + '"');
    }
}
