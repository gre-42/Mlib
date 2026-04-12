#pragma once
#include <Mlib/Macro_Executor/Boolean_Expression.hpp>
#include <Mlib/Macro_Executor/Json_Macro_Arguments.hpp>
#include <Mlib/Regex/Misc.hpp>
#include <Mlib/Strings/Utf8_Path.hpp>
#include <functional>
#include <map>
#include <string>
#include <vector>

namespace Mlib {

class MacroRecorder;
class SubstitutionMap;
class NotifyingJsonMacroArguments;
class JsonMacroArgumentsObserverToken;
class AssetReferences;
class JsonView;
class JsonMacroArgumentsAndLock;
class WritableJsonMacroArgumentsAndLock;

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
        const std::vector<Utf8Path>& search_path,
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
        JsonMacroArguments* local_json_macro_arguments) const;
    nlohmann::json eval(const std::string& expression) const;
    nlohmann::json eval(const std::string& expression, const JsonView& variables) const;
    template <class T>
    T eval(const std::string& expression) const;
    template <class T>
    T eval(const std::string& expression, const JsonView& variables) const;
    nlohmann::json at(const std::string& key) const;
    template <class T>
    T at(const std::string& key) const;
    JsonMacroArgumentsAndLock json_macro_arguments() const;
    WritableJsonMacroArgumentsAndLock writable_json_macro_arguments();
    bool eval(const BooleanExpression& expression) const;
    bool eval(const BooleanExpression& expression, const JsonView& variables) const;
    bool eval_boolean_expression(const nlohmann::json& j) const;
    bool eval_boolean_expression(const nlohmann::json& j, const JsonView& variables) const;
    JsonMacroArgumentsObserverToken add_observer(std::function<void()> func);
    JsonMacroArgumentsObserverToken add_finalizer(std::function<void()> func);
    JsonView block_arguments() const;
private:
    MacroRecorder& macro_recorder_;
    Utf8Path script_filename_;
    const std::vector<Utf8Path>& search_path_;
    JsonUserFunction json_user_function_;
    std::string context_;
    nlohmann::json block_arguments_;
    NotifyingJsonMacroArguments& global_json_macro_arguments_;
    const AssetReferences& asset_references_;
    bool verbose_;
};

}
