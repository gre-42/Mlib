#pragma once
#include <Mlib/Macro_Executor/Json_Macro_Arguments.hpp>
#include <Mlib/Regex/Misc.hpp>
#include <filesystem>
#include <functional>
#include <list>
#include <map>
#include <string>

namespace Mlib {

class MacroRecorder;
class SubstitutionMap;
class NotifyingJsonMacroArguments;
class AssetReferences;
struct FPath;
class JsonView;

class MacroLineExecutor {
    friend MacroRecorder;
public:
    typedef std::function<bool(
        const std::string& context,
        const MacroLineExecutor& macro_line_executor,
        const std::string& name,
        const JsonMacroArguments& arguments,
        JsonMacroArguments* local_json_macro_arguments)> JsonUserFunction;
    MacroLineExecutor(
        MacroRecorder& macro_recorder,
        std::string script_filename,
        const std::list<std::string>* search_path,
        JsonUserFunction json_user_function,
        std::string context,
        nlohmann::json block_arguments,
        NotifyingJsonMacroArguments& global_json_macro_arguments,
        const AssetReferences& asset_references,
        bool verbose);
    MacroLineExecutor changed_script_filename(
        std::string script_filename) const;
    MacroLineExecutor inserted_block_arguments(
        nlohmann::json block_arguments) const;
    MacroLineExecutor changed_context(
        std::string context,
        nlohmann::json block_arguments) const;
    MacroLineExecutor changed_script_filename_and_context(
        std::string script_filename,
        std::string context,
        nlohmann::json block_arguments) const;
    void operator () (
        const nlohmann::json& j,
        const JsonMacroArguments* caller_args,
        JsonMacroArguments* local_json_macro_arguments) const;
    nlohmann::json eval(const std::string& expression) const;
    nlohmann::json eval(const std::string& expression, const JsonView& variables) const;
    template <class T>
    T eval(const std::string& expression) const;
    template <class T>
    T eval(const std::string& expression, const JsonView& variables) const;
    void add_observer(std::function<void()> func);
    JsonView block_arguments() const;
private:
    MacroRecorder& macro_recorder_;
    std::string script_filename_;
    const std::list<std::string>* search_path_;
    JsonUserFunction json_user_function_;
    std::string context_;
    nlohmann::json block_arguments_;
    NotifyingJsonMacroArguments& global_json_macro_arguments_;
    const AssetReferences& asset_references_;
    bool verbose_;
};

}
