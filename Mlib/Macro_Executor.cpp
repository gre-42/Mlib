#include "Macro_Executor.hpp"
#include <Mlib/Regex.hpp>
#include <Mlib/String.hpp>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <regex>

namespace fs = std::filesystem;

using namespace Mlib;

void MacroExecutor::operator()(
    const std::string& script_filename,
    const std::string& working_directory,
    const std::function<bool(
        const std::function<std::string(const std::string&)>& fpath,
        const std::string& line)>& execute_user_function,
    SubstitutionString& substitutions,
    bool verbose)
{
    std::ifstream ifs{script_filename};
    const std::regex comment_reg("^(?:\\r?\\n|\\s)*#[\\S\\s]*$");
    const std::regex macro_begin_reg("^(?:\\r?\\n|\\s)*macro_begin ([\\w+-.]+)$");
    const std::regex macro_end_reg("^(?:\\r?\\n|\\s)*macro_end$");
    const std::regex macro_playback_reg("^(?:\\r?\\n|\\s)*macro_playback(?:\\r?\\n|\\s)+([\\w+-.]+)(" + substitute_pattern + ")$");
    const std::regex include_reg("^(?:\\r?\\n|\\s)*include ([\\w-. \\(\\)/+-]+)$");
    const std::regex empty_reg("^[\\s]*$");

    auto fpath = [&](const fs::path& f) -> std::string {
        if (f.empty()) {
            return "";
        } else if (f.string()[0] == '#') {
            return f.string().substr(1, f.string().length() - 1);
        } else if (f.is_absolute()) {
            return f.string();
        } else {
            return fs::weakly_canonical(fs::path(working_directory) / f).string();
        }
    };

    std::function<void(const std::string&, const std::string&)> process_line = [&](
        const std::string& line,
        const std::string& line_script_filename)
    {
        auto spath = [&](const fs::path& f) -> std::string {
            if (f.empty()) {
                return "";
            } else if (f.is_absolute()) {
                return f.string();
            } else {
                return fs::canonical(fs::path(line_script_filename).parent_path() / f).string();
            }
        };

        if (verbose) {
            std::cerr << "Processing line \"" << line << '"' << std::endl;
        }
        std::smatch match;
        if (std::regex_match(line, match, empty_reg)) {
            // Do nothing
        } else if (std::regex_match(line, match, comment_reg)) {
            // Do nothing
        } else if (std::regex_match(line, match, macro_playback_reg)) {
            auto macro_it = macros_.find(match[1].str());
            if (macro_it == macros_.end()) {
                throw std::runtime_error("No macro with name " + match[1].str() + " exists");
            }
            for(const std::string& l : macro_it->second.lines) {
                process_line(substitute(l, match[2].str()), macro_it->second.filename);
            }
        } else if (std::regex_match(line, match, include_reg)) {
            (*this)(
                spath(match[1].str()),
                working_directory,
                execute_user_function,
                substitutions,
                verbose);
        } else if (!execute_user_function(fpath, line)) {
            throw std::runtime_error("Could not parse line: \"" + line + '"');
        }
    };
    std::string line;
    std::list<std::pair<std::string, Macro>> recording_macros;
    while(std::getline(ifs, line, ';')) {
        if (line.length() > 0 && line[line.length() - 1] == '\r') {
            line = line.substr(0, line.length() - 1);
        }
        std::smatch match;

        if (std::regex_match(line, match, macro_begin_reg)) {
            recording_macros.push_back(std::make_pair(match[1].str(), Macro{filename: script_filename}));
        } else if (std::regex_match(line, match, macro_end_reg)) {
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
            process_line(substitutions.substitute(line), script_filename);
        }
    }

    if (!ifs.eof() && ifs.fail()) {
        throw std::runtime_error("Error reading from file: \"" + script_filename + '"');
    }
}
