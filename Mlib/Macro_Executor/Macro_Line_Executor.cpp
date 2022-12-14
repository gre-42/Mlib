#include "Macro_Line_Executor.hpp"
#include <Mlib/Env.hpp>
#include <Mlib/FPath.hpp>
#include <Mlib/Macro_Executor/Macro_Recorder.hpp>
#include <Mlib/Os/Os.hpp>
#include <Mlib/Regex.hpp>
#include <Mlib/Regex_Select.hpp>
#include <Mlib/Strings/To_Number.hpp>
#include <Mlib/Throw_Or_Abort.hpp>
#include <cereal/external/base64.hpp>
#include <filesystem>
#include <fstream>
#include <iostream>

namespace fs = std::filesystem;

using namespace Mlib;

static const std::string BASE64 = "__BASE64__";

static std::string encode_base64(const std::string& s) {
    return cereal::base64::encode((const unsigned char*)s.c_str(), s.size());
}

static std::string decode_base64(const std::string& s) {
    return cereal::base64::decode(s);
}

static std::string autoencode_base64(const std::string& s) {
    std::string encoded = encode_base64(s);
    encoded = encoded.substr(0, encoded.find("="));
    return BASE64 + encoded;
}

static std::string autodecode_base64(const std::string& s) {
    static const DECLARE_REGEX(re, "^" + BASE64 + "(\\w*)(.*)$");
    re::smatch match;
    if (Mlib::re::regex_match(s, match, re)) {
        return decode_base64(match[1].str()) + match[2].str();
    } else {
        return s;
    }
}

MacroLineExecutor::MacroLineExecutor(
    MacroRecorder& macro_file_executor,
    const std::string& script_filename,
    const std::list<std::string>& search_path,
    const UserFunction& user_function,
    const std::string& context,
    const SubstitutionMap& global_substitutions,
    bool verbose)
: macro_file_executor_{macro_file_executor},
  script_filename_{script_filename},
  search_path_{search_path},
  user_function_{user_function},
  context_{context},
  global_substitutions_{global_substitutions},
  verbose_{verbose}
{}

void MacroLineExecutor::operator () (
    const std::string& line,
    SubstitutionMap* local_substitutions,
    const RegexSubstitutionCache& rsc) const
{
    if (verbose_) {
        linfo() << "Processing line \"" << line << '"' << std::endl;
    }

    SubstitutionMap line_substitutions = global_substitutions_;
    if (!line_substitutions.insert("__DIR__", autoencode_base64(fs::path(script_filename_).parent_path().string()))) {
        THROW_OR_ABORT("__DIR__ variable already exists");
    }
    if (!line_substitutions.insert("__APPDATA__", autoencode_base64(get_appdata_directory()))) {
        THROW_OR_ABORT("__APPDATA__ variable already exists");
    }
    if (local_substitutions != nullptr) {
        line_substitutions.merge(*local_substitutions);
    }
    std::string subst_line = substitute_globals(line_substitutions.substitute(line, rsc), rsc);

    if (verbose_) {
        linfo() << "Substituted line: \"" << subst_line << '"' << std::endl;
    }

    static const DECLARE_REGEX(comment_reg, "^\\s*#[\\S\\s]*$");
    static const DECLARE_REGEX(macro_playback_reg, "^\\s*macro_playback\\s+([\\w+-.]+)(?:\\s+context=([\\w+-.]+))?(" + substitute_pattern + ")$");
    // static const DECLARE_REGEX(macro_playback_reg_fast, "^\\s*macro_playback\\s+([\\w+-.]+)(?:\\s+context=([\\w+-.]+))?([^;]*)$");
    static const DECLARE_REGEX(include_reg, "^\\s*include ([\\w+-. \\(\\)/]+)$");
    static const DECLARE_REGEX(empty_reg, "^[\\s]*$");

    auto fpathes = [&](const fs::path& f) -> std::list<std::string> {
        fs::path f_decoded = autodecode_base64(f.string());
        if (f_decoded.is_absolute()) {
            return { f_decoded.string() };
        } else {
            std::list<std::string> result;
            for (const std::string& wdir : search_path_) {
                auto path = fs::weakly_canonical(fs::path(wdir) / f);
                if (path_exists(path)) {
                    result.push_back(path.string());
                }
            }
            if (result.empty()) {
                THROW_OR_ABORT("Could not find path \"" + f.string() + "\" in search directories");
            }
            return result;
        }
    };

    auto fpath = [&](const fs::path& f) -> FPath {
        if (f.empty()) {
            return FPath{.is_variable = false, .path = ""};
        } else if (f.string()[0] == '#') {
            return FPath{.is_variable = true, .path = f.string().substr(1)};
        } else {
            fs::path f_decoded = autodecode_base64(f.string());
            if (f_decoded.is_absolute()) {
                return FPath{.is_variable = false, .path = f_decoded.string()};
            } else {
                for (const std::string& wdir : search_path_) {
                    auto path = fs::weakly_canonical(fs::path(wdir) / f);
                    if (path_exists(path)) {
                        return FPath{.is_variable = false, .path = path.string()};
                    }
                }
                THROW_OR_ABORT("Could not find path \"" + f.string() + "\" in search directories");
            }
        }
    };

    auto spath = [&](const fs::path& f) -> std::string {
        if (f.empty()) {
            THROW_OR_ABORT("Received empty script path");
        } else if (f.is_absolute()) {
            return f.string();
        } else {
            {
                auto local_path = fs::weakly_canonical(fs::path(script_filename_).parent_path() / f);
                if (path_exists(local_path)) {
                    return local_path.string();
                }
            }
            for (const std::string& wdir : search_path_) {
                auto path = fs::weakly_canonical(fs::path(wdir) / f);
                if (path_exists(path)) {
                    return path.string();
                }
            }
            THROW_OR_ABORT("Could not find path \"" + f.string() + "\" in script directory or search directories");
        }
    };

    Mlib::re::smatch match;
    if (Mlib::re::regex_match(subst_line, match, empty_reg)) {
        // Do nothing
    } else if (Mlib::re::regex_match(subst_line, match, comment_reg)) {
        // Do nothing
    } else if (Mlib::re::regex_match(subst_line, match, macro_playback_reg)) {
        std::string name = match[1].str();
        std::string context = match[2].str();
        SubstitutionMap local_substitutions2{replacements_to_map(match[3].str())};
        auto macro_it = macro_file_executor_.macros_.find(name);
        if (macro_it == macro_file_executor_.macros_.end()) {
            THROW_OR_ABORT("No macro with name " + name + " exists");
        }
        MacroLineExecutor mle2{
            macro_file_executor_,
            macro_it->second.filename,
            search_path_,
            user_function_,
            context.empty() ? context_ : context,
            global_substitutions_,
            verbose_};
        for (const std::string& l : macro_it->second.lines) {
            mle2(l, &local_substitutions2, rsc);
        }
    } else if (Mlib::re::regex_match(subst_line, match, include_reg)) {
        MacroLineExecutor mle2{
            macro_file_executor_,
            spath(match[1].str()),
            search_path_,
            user_function_,
            context_,
            global_substitutions_,
            verbose_};
        macro_file_executor_(mle2, rsc);
    } else {
        bool success = false;
        try {
            success = user_function_(context_, fpath, fpathes, *this, subst_line, local_substitutions);
        } catch (const std::exception& e) {
            auto msg = "Exception while processing line: \"" + subst_line + "\"\n\n" + e.what();
            if (verbose_) {
                linfo() << msg << std::endl;
            }
            throw std::runtime_error(msg);
        }
        if (!success) {
            auto msg = "Could not parse line: \"" + subst_line + '"';
            if (verbose_) {
                linfo() << msg << std::endl;
            }
            THROW_OR_ABORT(msg);
        }
    }
}

std::string MacroLineExecutor::substitute_globals(const std::string& str, const RegexSubstitutionCache& rsc) const {
    return macro_file_executor_.globals_.substitute(str, rsc);
}
