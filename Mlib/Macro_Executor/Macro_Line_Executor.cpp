#include "Macro_Line_Executor.hpp"
#include <Mlib/Env.hpp>
#include <Mlib/FPath.hpp>
#include <Mlib/Macro_Executor/Boolean_Expression.hpp>
#include <Mlib/Macro_Executor/Json_Expression.hpp>
#include <Mlib/Macro_Executor/Macro_Keys.hpp>
#include <Mlib/Macro_Executor/Macro_Recorder.hpp>
#include <Mlib/Macro_Executor/Notifying_Json_Macro_Arguments.hpp>
#include <Mlib/Os/Os.hpp>
#include <Mlib/Regex/Misc.hpp>
#include <Mlib/Regex/Regex_Select.hpp>
#include <Mlib/Strings/To_Number.hpp>
#include <Mlib/Threads/Recursion_Guard.hpp>
#include <Mlib/Threads/Thread_Local.hpp>
#include <Mlib/Throw_Or_Abort.hpp>
#include <Mlib/Time/Fps/Object_Life_Time.hpp>
#include <filesystem>
#include <fstream>
#include <iostream>

namespace fs = std::filesystem;

using namespace Mlib;

class PathResolver {
public:
    explicit PathResolver(
        const std::list<std::string>* search_path,
        std::string script_filename)
    : search_path_{search_path},
      script_filename_{std::move(script_filename)}
    {}
    std::list<std::string> fpathes(const std::filesystem::path& f) const {
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
    FPath fpath(const std::filesystem::path& f) const {
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
    std::string spath(const std::filesystem::path& f) const {
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
            THROW_OR_ABORT("Could not find relative path \"" + f.string() + "\" in script directory or search directories. Script: \"" + script_filename_ + '"');
        }
    }
private:
    const std::list<std::string>* search_path_;
    std::string script_filename_;
};

namespace DeclareMacroArgs {
BEGIN_ARGUMENT_LIST;
DECLARE_ARGUMENT(exclude);
DECLARE_ARGUMENT(required);
DECLARE_ARGUMENT(declare_macro);
DECLARE_ARGUMENT(content);
DECLARE_ARGUMENT(let);
}

MacroLineExecutor::MacroLineExecutor(
    MacroRecorder& macro_recorder,
    std::string script_filename,
    const std::list<std::string>* search_path,
    JsonUserFunction json_user_function,
    std::string context,
    nlohmann::json block_arguments,
    NotifyingJsonMacroArguments& global_json_macro_arguments,
    const AssetReferences& asset_references,
    bool verbose)
    : macro_recorder_{ macro_recorder }
    , script_filename_{ std::move(script_filename) }
    , search_path_{ search_path }
    , json_user_function_{ std::move(json_user_function) }
    , context_{ std::move(context) }
    , block_arguments_(std::move(block_arguments))
    , global_json_macro_arguments_{ global_json_macro_arguments }
    , asset_references_{ asset_references }
    , verbose_{ verbose }
{}

MacroLineExecutor MacroLineExecutor::changed_script_filename(
    std::string script_filename) const
{
    return MacroLineExecutor{
        macro_recorder_,
        std::move(script_filename),
        search_path_,
        json_user_function_,
        context_,
        block_arguments_,
        global_json_macro_arguments_,
        asset_references_,
        verbose_};
}

MacroLineExecutor MacroLineExecutor::inserted_block_arguments(
    const nlohmann::json& block_arguments) const
{
    JsonMacroArguments let{ block_arguments_ };
    let.insert_json(block_arguments);
    return MacroLineExecutor{
        macro_recorder_,
        script_filename_,
        search_path_,
        json_user_function_,
        context_,
        let.move_json(),
        global_json_macro_arguments_,
        asset_references_,
        verbose_};
}

MacroLineExecutor MacroLineExecutor::changed_context(
    std::string context,
    nlohmann::json block_arguments) const
{
    return MacroLineExecutor{
        macro_recorder_,
        script_filename_,
        search_path_,
        json_user_function_,
        std::move(context),
        std::move(block_arguments),
        global_json_macro_arguments_,
        asset_references_,
        verbose_};
}

MacroLineExecutor MacroLineExecutor::changed_script_filename_and_context(
    std::string script_filename,
    std::string context,
    nlohmann::json block_arguments) const
{
    return MacroLineExecutor{
        macro_recorder_,
        std::move(script_filename),
        search_path_,
        json_user_function_,
        std::move(context),
        std::move(block_arguments),
        global_json_macro_arguments_,
        asset_references_,
        verbose_};
}

void MacroLineExecutor::operator () (
    const nlohmann::json& j,
    JsonMacroArguments* local_json_macro_arguments) const
{
    // BENCHMARK static THREAD_LOCAL(RecursionCounter) recursion_counter = RecursionCounter{};
    // BENCHMARK RecursionGuard rg{ recursion_counter };
    // BENCHMARK std::list<std::pair<std::string, std::chrono::steady_clock::duration>> times;
    // BENCHMARK std::string short_description;
    // BENCHMARK ObjectLifeTime ot{ [&](std::chrono::steady_clock::duration elapsed) {
    // BENCHMARK     auto us = std::chrono::duration_cast<std::chrono::microseconds>(elapsed).count();
    // BENCHMARK     if ((us > 100) && !short_description.empty()) {
    // BENCHMARK         // linfo() << "Slow macro (" << us << " us): " << j;
    // BENCHMARK         linfo() << "Slow macro: " << std::string(recursion_counter, ' ') + short_description << " (" << us << " us)";
    // BENCHMARK         for (const auto& [d, e] : times) {
    // BENCHMARK             auto us1 = std::chrono::duration_cast<std::chrono::microseconds>(e).count();
    // BENCHMARK             linfo() << "Slow macro step: " << std::string(recursion_counter, ' ') + d << " (" << us1 << " us)";
    // BENCHMARK         }
    // BENCHMARK     }
    // BENCHMARK     } };
    if (verbose_) {
        linfo() << "Processing object " << j;
    }

    JsonMacroArguments merged_args{ block_arguments_ };
    // BENCHMARK times.emplace_back("merged_args", ot.elapsed());
    if (local_json_macro_arguments != nullptr) {
        // merged_args.merge(*local_json_macro_arguments);
        merged_args.insert_json(local_json_macro_arguments->json());
        // BENCHMARK times.emplace_back("local_json_macro_arguments", ot.elapsed());
    }
    if (j.type() == nlohmann::detail::value_t::object) {
        JsonView jv{ j };
        jv.validate(MacroKeys::options);
        // BENCHMARK times.emplace_back("validate", ot.elapsed());
        if ((int)jv.contains(MacroKeys::call) +
            (int)jv.contains(MacroKeys::declare_macro) +
            (int)jv.contains(MacroKeys::playback) +
            (int)jv.contains(MacroKeys::execute) +
            (int)jv.contains(MacroKeys::include) +
            (int)jv.contains(MacroKeys::comment) != 1)
        {
            std::stringstream msg;
            msg << j;
            THROW_OR_ABORT("Could not find exactly one out of call/declare_macro/playback/include/comment in \"" + msg.str() + '"');
        }
        auto global_args = global_json_macro_arguments_.json_macro_arguments();
        bool include = true;
        try {
            if (jv.contains(MacroKeys::required)) {
                BooleanExpression required;
                expression_from_json(jv.at(MacroKeys::required), required);
                include = eval(required);
            }
            // BENCHMARK times.emplace_back("required", ot.elapsed());
            if (include && jv.contains(MacroKeys::exclude)) {
                BooleanExpression exclude;
                expression_from_json(jv.at(MacroKeys::exclude), exclude);
                include = !eval(exclude);
            }
            // BENCHMARK times.emplace_back("exclude", ot.elapsed());
        } catch (const std::exception& e) {
            std::stringstream msg;
            msg << "Exception while evaluating conditionals in \"" << j << "\n\nException message: " << e.what();
            if (verbose_) {
                linfo() << msg.str();
            }
            throw std::runtime_error(msg.str());
        }
        if (include) {
            std::string context;
            if (auto c = jv.try_at<std::string>(MacroKeys::context); c.has_value()) {
                context = Mlib::eval<std::string>(*c, global_args, merged_args, asset_references_);
            } else {
                context = context_;
            }
            merged_args.insert_json("__DIR__", fs::path(script_filename_).parent_path().string());
            merged_args.insert_json("__APPDATA__", get_appdata_directory());
            PathResolver path_resolver{ search_path_, script_filename_ };
            auto insert_let = [&](JsonMacroArguments& let){
                try {
                    // BENCHMARK times.emplace_back("args", ot.elapsed());
                    if (jv.contains(MacroKeys::let)) {
                        let.insert_json(merged_args.subst_and_replace(jv.at(MacroKeys::let), global_args, asset_references_, SubstitutionMode::DEFAULT));
                    }
                    // BENCHMARK times.emplace_back("let", ot.elapsed());
                } catch (const std::exception& e) {
                    std::stringstream msg;
                    msg << "Exception while substituting variables for (0) " << std::setw(2) << j << "\n\nException message: " << e.what();
                    throw std::runtime_error(msg.str());
                }
            };
            // Note that "JsonMacroArguments::subst_and_replace" does not substitute "literals" and "content".
            // auto j_subst_raw = merged_args.subst_and_replace(j, global_args, asset_references_, SubstitutionMode::DEFAULT);
            // BENCHMARK times.emplace_back("subst", ot.elapsed());
            auto jv = JsonView{ j };
            if (jv.contains(MacroKeys::playback)) {
                if (jv.contains(MacroKeys::with)) {
                    std::stringstream msg;
                    msg << "\"with\" not supported for \"playback\": " << std::setw(2) << j;
                    THROW_OR_ABORT(msg.str());
                }
                auto without = jv.at<std::set<std::string>>(MacroKeys::without, std::set<std::string>());
                JsonMacroArguments let{ block_arguments_, Filter::without, without };
                insert_let(let);
                try {
                    // BENCHMARK times.emplace_back("args", ot.elapsed());
                    if (jv.contains(MacroKeys::arguments)) {
                        let.insert_json(merged_args.subst_and_replace(jv.at(MacroKeys::arguments), global_args, asset_references_, SubstitutionMode::ARGUMENT_COMPATIBILITY));
                    }
                    // BENCHMARK times.emplace_back("let", ot.elapsed());
                } catch (const std::exception& e) {
                    std::stringstream msg;
                    msg << "Exception while substituting variables for (1) " << std::setw(2) << j << "\n\nException message: " << e.what();
                    throw std::runtime_error(msg.str());
                }
                auto name = Mlib::eval<std::string>(
                    jv.at<std::string>(MacroKeys::playback),
                    global_args,
                    merged_args,
                    asset_references_);
                global_args.unlock();
                // BENCHMARK short_description = name;
                auto macro_it = macro_recorder_.json_macros_.find(name);
                if (macro_it == macro_recorder_.json_macros_.end()) {
                    THROW_OR_ABORT("No JSON macro with name " + name + " exists");
                }
                try {
                    let.insert_json(macro_it->second.block_arguments, Filter::without, without);
                } catch (const std::exception& e) {
                    std::stringstream msg;
                    msg << "Exception while merging \"let\" variables of macro \"" << name << "\". Line: " << std::setw(2) << macro_it->second.content << "\n\nException message: " << e.what();
                    if (verbose_) {
                        linfo() << msg.str();
                    }
                    throw std::runtime_error(msg.str());
                }
                auto mle2 = changed_script_filename_and_context(
                    macro_it->second.filename,
                    context,
                    let.move_json());
                // BENCHMARK times.emplace_back("macro fork", ot.elapsed());
                try {
                    mle2(macro_it->second.content, nullptr);
                } catch (const std::exception& e) {
                    std::stringstream msg;
                    msg << "Exception while executing macro \"" << name << "\". Line: " << std::setw(2) << macro_it->second.content << "\n\nException message: " << e.what();
                    if (verbose_) {
                        linfo() << msg.str();
                    }
                    throw std::runtime_error(msg.str());
                }
            } else if (jv.contains(MacroKeys::call)) {
                if (jv.contains(MacroKeys::with)) {
                    std::stringstream msg;
                    msg << "\"with\" not supported for \"call\": " << std::setw(2) << j;
                    THROW_OR_ABORT(msg.str());
                }
                auto without = jv.at<std::set<std::string>>(MacroKeys::without, std::set<std::string>());
                JsonMacroArguments let{ block_arguments_, Filter::without, without };
                insert_let(let);
                JsonMacroArguments args;
                args.set_fpathes([path_resolver](const std::filesystem::path& path){return path_resolver.fpathes(path);});
                args.set_fpath([path_resolver](const std::filesystem::path& path){return path_resolver.fpath(path);});
                args.set_spath([path_resolver](const std::filesystem::path& path){return path_resolver.spath(path);});
                try {
                    if (jv.contains(MacroKeys::arguments)) {
                        args.insert_json(merged_args.subst_and_replace(jv.at(MacroKeys::arguments), global_args, asset_references_, SubstitutionMode::DEFAULT));
                    }
                } catch (const std::exception& e) {
                    std::stringstream msg;
                    msg << "Exception while substituting variables for (2) " << std::setw(2) << j << "\n\nException message: " << e.what();
                    throw std::runtime_error(msg.str());
                }
                global_args.unlock();
                auto name = jv.at<std::string>(MacroKeys::call);
                // BENCHMARK short_description = name;
                auto mle2 = changed_context(
                    context,
                    let.move_json());
                // BENCHMARK times.emplace_back("call fork", ot.elapsed());
                bool success;
                try {
                    success = json_user_function_(
                        context,
                        mle2,
                        name,
                        args,
                        local_json_macro_arguments);
                } catch (const std::exception& e) {
                    std::stringstream msg;
                    msg << "Exception while executing function \"" << name << "\". Line: " << std::setw(2) << j << "\n\nException message: " << e.what();
                    if (verbose_) {
                        linfo() << msg.str();
                    }
                    throw std::runtime_error(msg.str());
                }
                // BENCHMARK times.emplace_back("call", ot.elapsed());
                if (!success) {
                    std::stringstream msg;
                    msg << "Could not find function with name \"" << name << "\". Line: " << std::setw(2) << j;
                    if (verbose_) {
                        linfo() << msg.str();
                    }
                    THROW_OR_ABORT(msg.str());
                }
            } else if (jv.contains(MacroKeys::execute)) {
                if (jv.contains(MacroKeys::with)) {
                    std::stringstream msg;
                    msg << "\"with\" not supported for \"execute\": " << std::setw(2) << j;
                    THROW_OR_ABORT(msg.str());
                }
                auto without = jv.at<std::set<std::string>>(MacroKeys::without, std::set<std::string>());
                JsonMacroArguments let{ block_arguments_, Filter::without, without };
                insert_let(let);
                global_args.unlock();
                if (jv.contains(MacroKeys::arguments)) {
                    THROW_OR_ABORT("\"execute\" does not support \"arguments\", use \"let\" instead");
                }
                auto mle2 = changed_context(context, let.json());
                // BENCHMARK times.emplace_back("execute fork", ot.elapsed());
                mle2(jv.at(MacroKeys::execute), nullptr);
            } else if (jv.contains(MacroKeys::include)) {
                if (jv.contains(MacroKeys::with)) {
                    std::stringstream msg;
                    msg << "\"with\" not supported for \"include\": " << std::setw(2) << j;
                    THROW_OR_ABORT(msg.str());
                }
                auto without = jv.at<std::set<std::string>>(MacroKeys::without, std::set<std::string>());
                JsonMacroArguments let{ block_arguments_, Filter::without, without };
                insert_let(let);
                global_args.unlock();
                if (jv.contains(MacroKeys::arguments)) {
                    THROW_OR_ABORT("\"include\" does not support \"arguments\", use \"let\" instead");
                }
                auto mle2 = changed_script_filename_and_context(
                    path_resolver.spath(jv.at<std::string>(MacroKeys::include)),
                    context,
                    let.json());
                // BENCHMARK times.emplace_back("include fork", ot.elapsed());
                macro_recorder_(mle2);
            } else if (jv.contains(MacroKeys::declare_macro)) {
                if (jv.contains(MacroKeys::without)) {
                    std::stringstream msg;
                    msg << "\"without\" not supported for \"declare_macro\": " << std::setw(2) << j;
                    THROW_OR_ABORT(msg.str());
                }
                auto with = jv.at<std::set<std::string>>(MacroKeys::with, std::set<std::string>());
                JsonMacroArguments let{ block_arguments_, Filter::with, with };
                insert_let(let);
                global_args.unlock();
                if (jv.contains(MacroKeys::arguments)) {
                    THROW_OR_ABORT("\"declare_macro\" does not support \"arguments\", use \"let\" instead");
                }
                try {
                    jv.validate(DeclareMacroArgs::options);
                } catch (const std::runtime_error& e) {
                    throw std::runtime_error((std::stringstream() << e.what() << "\nCould not validate " << jv.json()).str());
                }
                auto name = jv.at<std::string>(MacroKeys::declare_macro);
                if (verbose_) {
                    linfo() << "Storing macro \"" << name << '"';
                }
                if (!macro_recorder_.json_macros_.try_emplace(
                    name,
                    JsonMacro{
                        .filename = script_filename_,
                        .content = jv.at(DeclareMacroArgs::content),
                        .block_arguments = let.json()
                    }).second)
                {
                    THROW_OR_ABORT("Macro with name \"" + name + "\" already exists");
                }
            } else if (jv.contains(MacroKeys::comment)) {
                // Do nothing
            } else {
                std::stringstream msg;
                msg << j;
                THROW_OR_ABORT("Cannot interpret " + msg.str());
            }
        }
    } else if (j.type() == nlohmann::detail::value_t::array) {
        JsonMacroArguments local_json_macro_arguments_2;
        for (const nlohmann::json& l : j) {
            (*this)(l, &local_json_macro_arguments_2);
        }
    } else {
        std::stringstream msg;
        msg << j;
        THROW_OR_ABORT("Not object or array: \"" + msg.str() + '"');
    }
}

nlohmann::json MacroLineExecutor::eval(const std::string& expression, const JsonView& variables) const {
    auto global_args = global_json_macro_arguments_.json_macro_arguments();
    return Mlib::eval(expression, global_args, variables, asset_references_);
}

nlohmann::json MacroLineExecutor::eval(const std::string& expression) const {
    auto global_args = global_json_macro_arguments_.json_macro_arguments();
    return Mlib::eval(expression, global_args, asset_references_);
}

template <class T>
T MacroLineExecutor::eval(const std::string& expression, const JsonView& variables) const {
    auto global_args = global_json_macro_arguments_.json_macro_arguments();
    return Mlib::eval<T>(expression, global_args, variables, asset_references_);
}

template <class T>
T MacroLineExecutor::eval(const std::string& expression) const {
    auto global_args = global_json_macro_arguments_.json_macro_arguments();
    return Mlib::eval<T>(expression, global_args, JsonView{ block_arguments_ }, asset_references_);
}

bool MacroLineExecutor::eval(
    const std::vector<std::vector<std::string>>& expression) const
{
    for (const auto& rs : expression) {
        bool ok = false;
        for (const auto& r : rs) {
            if (eval<bool>(r)) {
                ok = true;
                break;
            }
        }
        if (!ok) {
            return false;
        }
    }
    return true;
}

bool MacroLineExecutor::eval(
    const std::vector<std::vector<std::string>>& expression,
    const JsonView& variables) const
{
    for (const auto& rs : expression) {
        bool ok = false;
        for (const auto& r : rs) {
            if (eval<bool>(r, variables)) {
                ok = true;
                break;
            }
        }
        if (!ok) {
            return false;
        }
    }
    return true;
}

JsonMacroArgumentsObserverToken MacroLineExecutor::add_observer(std::function<void()> func) {
    return global_json_macro_arguments_.add_observer(std::move(func));
}

JsonView MacroLineExecutor::block_arguments() const {
    return JsonView{ block_arguments_ };
}

template bool MacroLineExecutor::eval<bool>(const std::string& expression, const JsonView& variables) const;
template bool MacroLineExecutor::eval<bool>(const std::string& expression) const;
template std::string MacroLineExecutor::eval<std::string>(const std::string& expression, const JsonView& variables) const;
template std::string MacroLineExecutor::eval<std::string>(const std::string& expression) const;
