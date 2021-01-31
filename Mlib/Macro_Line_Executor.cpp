#include "Macro_Line_Executor.hpp"
#include <Mlib/Macro_Recorder.hpp>
#include <Mlib/Regex_Select.hpp>
#include <Mlib/Regex.hpp>
#include <Mlib/String.hpp>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <regex>

namespace fs = std::filesystem;

using namespace Mlib;

MacroLineExecutor::MacroLineExecutor(
    MacroRecorder& macro_file_executor,
    const std::string& script_filename,
    const std::string& working_directory,
    const UserFunction& user_function,
    const std::string& context,
    const SubstitutionString& substitutions,
    bool verbose)
: macro_file_executor_{macro_file_executor},
  script_filename_{script_filename},
  working_directory_{working_directory},
  user_function_{user_function},
  context_{context},
  substitutions_{substitutions},
  verbose_{verbose}
{}

void MacroLineExecutor::operator () (
    const std::string& line,
    const RegexSubstitutionCache& rsc) const
{
    std::string subst_line = substitutions_.substitute(line, rsc);
    static const DECLARE_REGEX(comment_reg, "^\\s*#[\\S\\s]*$");
    static const DECLARE_REGEX(macro_playback_reg, "^\\s*macro_playback\\s+([\\w+-.]+)(?:\\s+context=([\\w+-.]+))?(" + substitute_pattern + ")$");
    // static const DECLARE_REGEX(macro_playback_reg_fast, "^\\s*macro_playback\\s+([\\w+-.]+)(?:\\s+context=([\\w+-.]+))?([^;]*)$");
    static const DECLARE_REGEX(include_reg, "^\\s*include ([\\w-. \\(\\)/+-]+)$");
    static const DECLARE_REGEX(empty_reg, "^[\\s]*$");

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
        std::cerr << "Processing line \"" << subst_line << '"' << std::endl;
    }
    Mlib::re::smatch match;
    if (Mlib::re::regex_match(subst_line, match, empty_reg)) {
        // Do nothing
    } else if (Mlib::re::regex_match(subst_line, match, comment_reg)) {
        // Do nothing
    } else if (Mlib::re::regex_match(subst_line, match, macro_playback_reg)) {
        std::string name = match[1].str();
        std::string context = match[2].str();
        std::string subst_pattern = match[3].str();
        auto macro_it = macro_file_executor_.macros_.find(name);
        if (macro_it == macro_file_executor_.macros_.end()) {
            throw std::runtime_error("No macro with name " + name + " exists");
        }
        MacroLineExecutor mle2{
            macro_file_executor_,
            macro_it->second.filename,
            working_directory_,
            user_function_,
            context.empty() ? context_ : context,
            substitutions_,
            verbose_};
        for (const std::string& l : macro_it->second.lines) {
            mle2(substitute(l, subst_pattern, rsc), rsc);
        }
    } else if (Mlib::re::regex_match(subst_line, match, include_reg)) {
        MacroLineExecutor mle2{
            macro_file_executor_,
            spath(match[1].str()),
            working_directory_,
            user_function_,
            context_,
            substitutions_,
            verbose_};
        macro_file_executor_(mle2, rsc);
    } else {
        bool success = false;
        try {
            success = user_function_(context_, fpath, *this, subst_line);
        } catch (const std::runtime_error& e) {
            throw std::runtime_error("Exception while processing line: \"" + subst_line + "\"\n\n" + e.what());
        }
        if (!success) {
            throw std::runtime_error("Could not parse line: \"" + subst_line + '"');
        }
        
    }
}
