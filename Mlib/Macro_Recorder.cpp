#include "Macro_Recorder.hpp"
#include <Mlib/Macro_Line_Executor.hpp>
#include <fstream>
#include <regex>

using namespace Mlib;

void MacroRecorder::operator()(const MacroLineExecutor& macro_line_executor, const RegexSubstitutionCache& rsc)
{
    std::ifstream ifs{macro_line_executor.script_filename_};
    static const DECLARE_REGEX(macro_begin_reg, "^\\s*macro_begin ([\\w+-.]+)$");
    static const DECLARE_REGEX(macro_end_reg, "^\\s*macro_end$");

    std::string line;
    std::list<std::pair<std::string, Macro>> recording_macros;
    while(std::getline(ifs, line, ';')) {
        if (line.length() > 0 && line[line.length() - 1] == '\r') {
            line = line.substr(0, line.length() - 1);
        }
        Mlib::re::smatch match;

        if (Mlib::re::regex_match(line, match, macro_begin_reg)) {
            recording_macros.push_back(std::make_pair(match[1].str(), Macro{.filename = macro_line_executor.script_filename_}));
        } else if (Mlib::re::regex_match(line, match, macro_end_reg)) {
            if (recording_macros.empty()) {
                throw std::runtime_error("Macro-end despite no active macro");
            }
            auto it = macros_.insert(std::make_pair(recording_macros.back().first, recording_macros.back().second));
            if (!it.second) {
                throw std::runtime_error("Duplicate macro: " + recording_macros.back().first);
            }
            recording_macros.pop_back();
        } else if (!recording_macros.empty()) {
            recording_macros.back().second.lines.push_back(line);
        } else {
            macro_line_executor(line, SubstitutionString(), rsc);
        }
    }

    if (!ifs.eof() && ifs.fail()) {
        throw std::runtime_error("Error reading from file: \"" + macro_line_executor.script_filename_ + '"');
    }

    if (!recording_macros.empty()) {
        throw std::runtime_error("Missing macro_end; in file \"" + macro_line_executor.script_filename_ + '"');
    }
}
