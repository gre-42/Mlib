#include "Macro_Executor.hpp"
#include <Mlib/Regex.hpp>
#include <Mlib/String.hpp>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <regex>

namespace fs = std::filesystem;

using namespace Mlib;

void MacroFileExecutor::operator()(const MacroLineExecutor& lp)
{
    std::ifstream ifs{lp.script_filename_};
    const std::regex macro_begin_reg("^(?:\\r?\\n|\\s)*macro_begin ([\\w+-.]+)$");
    const std::regex macro_end_reg("^(?:\\r?\\n|\\s)*macro_end$");

    std::string line;
    std::list<std::pair<std::string, Macro>> recording_macros;
    while(std::getline(ifs, line, ';')) {
        if (line.length() > 0 && line[line.length() - 1] == '\r') {
            line = line.substr(0, line.length() - 1);
        }
        std::smatch match;

        if (std::regex_match(line, match, macro_begin_reg)) {
            recording_macros.push_back(std::make_pair(match[1].str(), Macro{filename: lp.script_filename_}));
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
            lp(lp.substitutions_.substitute(line));
        }
    }

    if (!ifs.eof() && ifs.fail()) {
        throw std::runtime_error("Error reading from file: \"" + lp.script_filename_ + '"');
    }
}

MacroLineExecutor::MacroLineExecutor(
    MacroFileExecutor& macro_file_executor,
    const std::string& script_filename,
    const std::string& working_directory,
    MacroFileExecutor::UserFunction& execute_user_function,
    SubstitutionString& substitutions,
    bool verbose)
: macro_file_executor_{macro_file_executor},
  script_filename_{script_filename},
  working_directory_{working_directory},
  execute_user_function_{execute_user_function},
  substitutions_{substitutions},
  verbose_{verbose}
{}

void MacroLineExecutor::operator () (const std::string& line) const
{
    const std::regex comment_reg("^(?:\\r?\\n|\\s)*#[\\S\\s]*$");
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
            return fs::weakly_canonical(fs::path(working_directory_) / f).string();
        }
    };

    auto spath = [&](const fs::path& f) -> std::string {
        if (f.empty()) {
            return "";
        } else if (f.is_absolute()) {
            return f.string();
        } else {
            return fs::canonical(fs::path(script_filename_).parent_path() / f).string();
        }
    };

    if (verbose_) {
        std::cerr << "Processing line \"" << line << '"' << std::endl;
    }
    std::smatch match;
    if (std::regex_match(line, match, empty_reg)) {
        // Do nothing
    } else if (std::regex_match(line, match, comment_reg)) {
        // Do nothing
    } else if (std::regex_match(line, match, macro_playback_reg)) {
        auto macro_it = macro_file_executor_.macros_.find(match[1].str());
        if (macro_it == macro_file_executor_.macros_.end()) {
            throw std::runtime_error("No macro with name " + match[1].str() + " exists");
        }
        MacroLineExecutor lp2{
            macro_file_executor_,
            macro_it->second.filename,
            working_directory_,
            execute_user_function_,
            substitutions_,
            verbose_};
        for(const std::string& l : macro_it->second.lines) {
            lp2(substitute(l, match[2].str()));
        }
    } else if (std::regex_match(line, match, include_reg)) {
        MacroLineExecutor lp2{
            macro_file_executor_,
            spath(match[1].str()),
            working_directory_,
            execute_user_function_,
            substitutions_,
            verbose_};
        macro_file_executor_(lp2);
    } else if (!execute_user_function_(fpath, *this, line)) {
        throw std::runtime_error("Could not parse line: \"" + line + '"');
    }
}
