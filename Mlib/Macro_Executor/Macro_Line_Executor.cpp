#include "Macro_Line_Executor.hpp"
#include <Mlib/Env.hpp>
#include <Mlib/FPath.hpp>
#include <Mlib/Macro_Executor/MacroKeys.hpp>
#include <Mlib/Macro_Executor/Macro_Recorder.hpp>
#include <Mlib/Macro_Executor/Notifying_Json_Macro_Arguments.hpp>
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

namespace IncludeLiterals {
BEGIN_ARGUMENT_LIST;
}

MacroLineExecutor::MacroLineExecutor(
    MacroRecorder& macro_recorder,
    std::string script_filename,
    const std::list<std::string>* search_path,
    JsonUserFunction json_user_function,
    std::string context,
    const NotifyingJsonMacroArguments& global_json_macro_arguments,
    bool verbose)
: macro_recorder_{macro_recorder},
  script_filename_{std::move(script_filename)},
  search_path_{search_path},
  json_user_function_{std::move(json_user_function)},
  context_{std::move(context)},
  global_json_macro_arguments_{global_json_macro_arguments},
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
        context_,
        global_json_macro_arguments_,
        verbose_};
}

void MacroLineExecutor::operator () (
    const nlohmann::json& j,
    const JsonMacroArguments* caller_args,
    JsonMacroArguments* local_json_macro_arguments) const
{
    if (verbose_) {
        linfo() << "Processing object \"" << j << '"';
    }

    JsonMacroArguments merged_args;
    if (caller_args != nullptr) {
        merged_args.merge(*caller_args);
    }
    if (local_json_macro_arguments != nullptr) {
        merged_args.merge(*local_json_macro_arguments);
    }
    if (j.type() == nlohmann::detail::value_t::object) {
        validate(j, MacroKeys::options);
        if ((int)j.contains(MacroKeys::call) +
            (int)j.contains(MacroKeys::playback) +
            (int)j.contains(MacroKeys::include) +
            (int)j.contains(MacroKeys::comment) != 1)
        {
            std::stringstream msg;
            msg << j;
            THROW_OR_ABORT("Could not find exactly out of call/playback/include/comment in \"" + msg.str() + '"');
        }
        std::string context = j.contains(MacroKeys::context)
            ? j.at(MacroKeys::context).get<std::string>()
            : context_;
        bool include = true;
        if (j.contains(MacroKeys::required)) {
            for (const auto& e : j.at(MacroKeys::required).get<std::vector<std::string>>()) {
                auto g = global_json_macro_arguments_.json_macro_arguments().try_at<bool>(e);
                if (g.has_value()) {
                    if (!g.value()) {
                        include = false;
                        break;
                    }
                } else if (!merged_args.at<bool>(e)) {
                    include = false;
                    break;
                }
            }
        }
        if (j.contains(MacroKeys::exclude)) {
            for (const auto& e : j.at(MacroKeys::exclude).get<std::vector<std::string>>()) {
                auto g = global_json_macro_arguments_.json_macro_arguments().try_at<bool>(e);
                if (g.has_value()) {
                    if (g.value()) {
                        include = false;
                        break;
                    }
                } else if (merged_args.at<bool>(e)) {
                    include = false;
                    break;
                }
            }
        }
        if (include) {
            merged_args.merge(global_json_macro_arguments_.json_macro_arguments());
            merged_args.insert_json("__DIR__", fs::path(script_filename_).parent_path().string());
            merged_args.insert_json("__APPDATA__", get_appdata_directory());
            JsonMacroArguments args;
            args.set_fpathes([this](const std::filesystem::path& path){return fpathes(path);});
            args.set_fpath([this](const std::filesystem::path& path){return fpath(path);});
            args.set_spath([this](const std::filesystem::path& path){return spath(path);});
            {
                if (j.contains(MacroKeys::literals)) {
                    args.insert_json(merged_args.subst_and_replace(j.at(MacroKeys::literals)));
                }
                if (j.contains(MacroKeys::__DIR__)) {
                    for (const auto& [key, value] : j.at(MacroKeys::__DIR__).items()) {
                        args.insert_json(key, (fs::path(script_filename_).parent_path() / merged_args.subst_and_replace(value)).string());
                    }
                }
                if (j.contains(MacroKeys::__APPDATA__)) {
                    for (const auto& [key, value] : j.at(MacroKeys::__APPDATA__).items()) {
                        args.insert_json(key, (fs::path{get_appdata_directory()} / merged_args.subst_and_replace(value)).string());
                    }
                }
            }
            if (j.contains(MacroKeys::playback)) {
                std::string name = merged_args.subst_and_replace(j.at(MacroKeys::playback));
                auto macro_it = macro_recorder_.json_macros_.find(name);
                if (macro_it == macro_recorder_.json_macros_.end()) {
                    THROW_OR_ABORT("No JSON macro with name " + name + " exists");
                }
                MacroLineExecutor mle2{
                    macro_recorder_,
                    macro_it->second.filename,
                    search_path_,
                    json_user_function_,
                    context,
                    global_json_macro_arguments_,
                    verbose_};
                if (macro_it->second.content.type() != nlohmann::detail::value_t::array) {
                    THROW_OR_ABORT("Macro is not an array: \"" + name + '"');
                }
                JsonMacroArguments local_json_macro_arguments_2;
                for (const json& l : macro_it->second.content) {
                    mle2(l, &args, &local_json_macro_arguments_2);
                }
            } else if (j.contains(MacroKeys::call)) {
                std::string name = merged_args.subst_and_replace(j.at(MacroKeys::call));
                bool success;
                try {
                    success = json_user_function_(
                        context,
                        *this,
                        name,
                        args,
                        local_json_macro_arguments);
                } catch (const std::exception& e) {
                    std::stringstream msg;
                    msg << "Exception while executing function \"" << name << "\". Line: \"" << j << "\"\n\n" << e.what();
                    if (verbose_) {
                        linfo() << msg.str();
                    }
                    throw std::runtime_error(msg.str());
                }
                if (!success) {
                    std::stringstream msg;
                    msg << "Could not find function with name \"" << name << "\". Line \"" << j << '"';
                    if (verbose_) {
                        linfo() << msg.str();
                    }
                    THROW_OR_ABORT(msg.str());
                }
            } else if (j.contains(MacroKeys::include)) {
                args.validate(IncludeLiterals::options);
                MacroLineExecutor mle2{
                    macro_recorder_,
                    spath(j.at(MacroKeys::include)),
                    search_path_,
                    json_user_function_,
                    context_,
                    global_json_macro_arguments_,
                    verbose_};
                macro_recorder_(mle2);
            } else if (j.contains(MacroKeys::comment)) {
                // Do nothing
            } else {
                std::stringstream msg;
                msg << j;
                THROW_OR_ABORT("Cannot interpret " + msg.str());
            }
        }
    } else {
        std::stringstream msg;
        msg << j;
        THROW_OR_ABORT("Not an object " + msg.str());
    }
}

std::list<std::string> MacroLineExecutor::fpathes(const fs::path& f) const {
    if (f.is_absolute()) {
        return { f.string() };
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
        if (f.is_absolute()) {
            return FPath{.is_variable = false, .path = f.string()};
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
