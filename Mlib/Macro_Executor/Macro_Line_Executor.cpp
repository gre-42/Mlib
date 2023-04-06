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

using json = nlohmann::json;
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
    encoded = encoded.substr(0, encoded.find('='));
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
    MacroRecorder& macro_recorder,
    std::string script_filename,
    const std::list<std::string>* search_path,
    JsonUserFunction json_user_function,
    UserFunction user_function,
    std::string context,
    const NotifyingSubstitutionMap& global_substitutions,
    bool verbose)
: macro_recorder_{macro_recorder},
  script_filename_{std::move(script_filename)},
  search_path_{search_path},
  json_user_function_{std::move(json_user_function)},
  user_function_{std::move(user_function)},
  context_{std::move(context)},
  global_substitutions_{global_substitutions},
  verbose_{verbose}
{}

MacroLineExecutor MacroLineExecutor::changed_script_filename(
    std::string script_filename) const
{
    return MacroLineExecutor{
        macro_recorder_,
        script_filename,
        search_path_,
        json_user_function_,
        user_function_,
        context_,
        global_substitutions_,
        verbose_};
}

void MacroLineExecutor::operator () (
    const nlohmann::json& j,
    const JsonMacroArguments* caller_args) const
{
    if (verbose_) {
        linfo() << "Processing object \"" << j << '"';
    }

    if (j.type() == nlohmann::detail::value_t::object) {
        JsonMacroArguments args;
        if (j.contains("literals")) {
            args.insert_json(j.at("literals"));
        }
        if (j.contains("scripts")) {
            for (const auto& [key, value] : j.at("scripts").items()) {
                args.insert_path(key, spath(value));
            }
        }
        if (j.contains("pathes")) {
            for (const auto& [key, value] : j.at("pathes").items()) {
                args.insert_path(key, fpath(value).path);
            }
        }
        if (j.contains("path_lists")) {
            for (const auto& [key, value] : j.at("path_lists").items()) {
                args.insert_path_list(key, fpathes(value));
            }
        }
        if (j.contains("__DIR__")) {
            for (const auto& [key, value] : j.at("__DIR__").items()) {
                args.insert_path(key, fs::path(script_filename_).parent_path() / value);
            }
        }
        if (j.contains("__APPDATA__")) {
            for (const auto& [key, value] : j.at("__APPDATA__").items()) {
                args.insert_path(key, fs::path{get_appdata_directory()} / value);
            }
        }
        if ((caller_args != nullptr) && j.contains("input")) {
            for (const auto& [k, v] : j.at("input").items()) {
                std::string vs = v;
                if (caller_args->contains_json(vs)) {
                    args.insert_json(k, caller_args->at(vs));
                    continue;
                }
                if (caller_args->contains_path(vs)) {
                    args.insert_path(k, caller_args->path(vs));
                    continue;
                }
                if (caller_args->contains_path_list(vs)) {
                    args.insert_path_list(k, caller_args->path_list(vs));
                    continue;
                }
                THROW_OR_ABORT("Could not find input variable \"" + vs + '"');
            }
        }
        if (j.contains("playback")) {
            std::string name = j.at("playback");
            std::string context = j.contains("context") ? j.at("context") : "";
            auto macro_it = macro_recorder_.json_macros_.find(name);
            if (macro_it == macro_recorder_.json_macros_.end()) {
                THROW_OR_ABORT("No JSON macro with name " + name + " exists");
            }
            MacroLineExecutor mle2{
                macro_recorder_,
                macro_it->second.filename,
                search_path_,
                json_user_function_,
                user_function_,
                context.empty() ? context_ : context,
                global_substitutions_,
                verbose_};
            if (macro_it->second.content.type() != nlohmann::detail::value_t::array) {
                THROW_OR_ABORT("Macro is not an array: \"" + name + '"');
            }
            for (const json& l : macro_it->second.content) {
                mle2(l, &args);
            }
        } else if (j.contains("call")) {
            bool success;
            try {
                success = json_user_function_(
                    context_,
                    *this,
                    j.at("call"),
                    args);
            } catch (const std::exception& e) {
                std::stringstream msg;
                msg << "Exception while processing line: \"" << j << "\"\n\n" << e.what();
                if (verbose_) {
                    linfo() << msg.str();
                }
                throw std::runtime_error(msg.str());
            }
            if (!success) {
                std::stringstream msg;
                msg << "Could not parse line: \"" << j << '"';
                if (verbose_) {
                    linfo() << msg.str();
                }
                THROW_OR_ABORT(msg.str());
            }
        } else {
            std::stringstream msg;
            msg << j;
            THROW_OR_ABORT("Cannot interpret " + msg.str());
        }
    } else {
        std::stringstream msg;
        msg << j;
        THROW_OR_ABORT("Not an object " + msg.str());
    }
}

void MacroLineExecutor::operator () (
    const std::string& line,
    SubstitutionMap* local_substitutions) const
{
    if (verbose_) {
        linfo() << "Processing line \"" << line << '"';
    }

    SubstitutionMap line_substitutions = global_substitutions_.substitution_map();
    if (!line_substitutions.insert("__DIR__", autoencode_base64(fs::path(script_filename_).parent_path().string()))) {
        THROW_OR_ABORT("__DIR__ variable already exists");
    }
    if (!line_substitutions.insert("__APPDATA__", autoencode_base64(get_appdata_directory()))) {
        THROW_OR_ABORT("__APPDATA__ variable already exists");
    }
    if (local_substitutions != nullptr) {
        line_substitutions.merge(*local_substitutions);
    }
    std::string subst_line = substitute_globals(line_substitutions.substitute(line));

    if (verbose_) {
        linfo() << "Substituted line: \"" << subst_line << '"';
    }

    static const DECLARE_REGEX(comment_reg, "^\\s*#[\\S\\s]*$");
    static const DECLARE_REGEX(macro_playback_reg, "^\\s*macro_playback\\s+([\\w+-.]+)(?:\\s+context=([\\w+-.]+))?(" + substitute_pattern + ")$");
    // static const DECLARE_REGEX(macro_playback_reg_fast, "^\\s*macro_playback\\s+([\\w+-.]+)(?:\\s+context=([\\w+-.]+))?([^;]*)$");
    static const DECLARE_REGEX(include_reg, "^\\s*include ([\\w+-. \\(\\)/]+)$");
    static const DECLARE_REGEX(empty_reg, "^[\\s]*$");

    Mlib::re::smatch match;
    if (Mlib::re::regex_match(subst_line, match, empty_reg) ||
        Mlib::re::regex_match(subst_line, match, comment_reg))
    {
        // Do nothing
    } else if (Mlib::re::regex_match(subst_line, match, macro_playback_reg)) {
        std::string name = match[1].str();
        std::string context = match[2].str();
        SubstitutionMap local_substitutions2{replacements_to_map(match[3].str())};
        auto macro_it = macro_recorder_.text_macros_.find(name);
        if (macro_it == macro_recorder_.text_macros_.end()) {
            THROW_OR_ABORT("No text macro with name " + name + " exists");
        }
        MacroLineExecutor mle2{
            macro_recorder_,
            macro_it->second.filename,
            search_path_,
            json_user_function_,
            user_function_,
            context.empty() ? context_ : context,
            global_substitutions_,
            verbose_};
        for (const std::string& l : macro_it->second.lines) {
            mle2(l, &local_substitutions2);
        }
    } else if (Mlib::re::regex_match(subst_line, match, include_reg)) {
        MacroLineExecutor mle2{
            macro_recorder_,
            spath(match[1].str()),
            search_path_,
            json_user_function_,
            user_function_,
            context_,
            global_substitutions_,
            verbose_};
        macro_recorder_(mle2);
    } else {
        bool success;
        try {
            success = user_function_(
                context_,
                std::bind(&MacroLineExecutor::spath, this, std::placeholders::_1),
                std::bind(&MacroLineExecutor::fpath, this, std::placeholders::_1),
                std::bind(&MacroLineExecutor::fpathes, this, std::placeholders::_1),
                *this,
                subst_line,
                local_substitutions);
        } catch (const std::exception& e) {
            auto msg = "Exception while processing line: \"" + subst_line + "\"\n\n" + e.what();
            if (verbose_) {
                linfo() << msg;
            }
            throw std::runtime_error(msg);
        }
        if (!success) {
            auto msg = "Could not parse line: \"" + subst_line + '"';
            if (verbose_) {
                linfo() << msg;
            }
            THROW_OR_ABORT(msg);
        }
    }
}

std::string MacroLineExecutor::substitute_globals(const std::string& str) const {
    return macro_recorder_.globals_.substitute(str);
}

std::list<std::string> MacroLineExecutor::fpathes(const fs::path& f) const {
    fs::path f_decoded = autodecode_base64(f.string());
    if (f_decoded.is_absolute()) {
        return { f_decoded.string() };
    } else {
        std::list<std::string> result;
        for (const std::string& wdir : *search_path_) {
            auto path = fs::weakly_canonical(fs::path(wdir) / f);
            if (path_exists(path)) {
                result.push_back(path.string());
            }
        }
        if (result.empty()) {
            THROW_OR_ABORT("Could not find a single relative path \"" + f.string() + "\" in search directories");
        }
        return result;
    }
}

FPath MacroLineExecutor::fpath(const fs::path& f) const {
    if (f.empty()) {
        return FPath{.is_variable = false, .path = ""};
    } else if (f.string()[0] == '#') {
        return FPath{.is_variable = true, .path = f.string().substr(1)};
    } else {
        fs::path f_decoded = autodecode_base64(f.string());
        if (f_decoded.is_absolute()) {
            return FPath{.is_variable = false, .path = f_decoded.string()};
        } else {
            for (const std::string& wdir : *search_path_) {
                auto path = fs::weakly_canonical(fs::path(wdir) / f);
                if (path_exists(path)) {
                    return FPath{.is_variable = false, .path = path.string()};
                }
            }
            THROW_OR_ABORT("Could not find relative path \"" + f.string() + "\" in search directories");
        }
    }
}

std::string MacroLineExecutor::spath(const fs::path& f) const {
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
        for (const std::string& wdir : *search_path_) {
            auto path = fs::weakly_canonical(fs::path(wdir) / f);
            if (path_exists(path)) {
                return path.string();
            }
        }
        THROW_OR_ABORT("Could not find relative path \"" + f.string() + "\" in script directory or search directories");
    }
}
