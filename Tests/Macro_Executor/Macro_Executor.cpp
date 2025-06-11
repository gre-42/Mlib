#include <Mlib/Env.hpp>
#include <Mlib/Macro_Executor/Asset_References.hpp>
#include <Mlib/Macro_Executor/Json_Expression.hpp>
#include <Mlib/Macro_Executor/Macro_Line_Executor.hpp>
#include <Mlib/Macro_Executor/Macro_Recorder.hpp>
#include <Mlib/Macro_Executor/Notifying_Json_Macro_Arguments.hpp>
#include <iostream>

using namespace Mlib;

void test_json() {
    MacroLineExecutor::JsonUserFunction json_user_function = [](
        const std::string& context,
        const MacroLineExecutor& macro_line_executor,
        const std::string& name,
        const JsonMacroArguments& arguments,
        JsonMacroArguments* local_json_macro_arguments)
    {
        linfo() << "Name: " << name;
        linfo() << "Arguments: " << arguments;
        return true;
    };
    std::list<std::string> search_path{"."};
    NotifyingJsonMacroArguments global_substitutions;
    AssetReferences asset_references;
    MacroRecorder mr;
    MacroLineExecutor mle{
        mr,
        "script.scn.json",
        &search_path,
        json_user_function,
        "context",
        nlohmann::json::object(),
        global_substitutions,
        asset_references,
        true};
    mr(mle);
}

void test_eval() {
    linfo() << "eval " << eval<bool>("'hello' == 'world'", JsonView{ nlohmann::json::object() });
    linfo() << "eval " << eval<bool>("'hello' != 'world'", JsonView{ nlohmann::json::object() });
    linfo() << "eval " << eval<bool>("%%levels/aircraft_carrier0/game_modes == 'hello'", JsonView{ nlohmann::json::object() });
}

int main(int argc, char** argv) {
#ifndef __ANDROID__
    set_app_reldir("macro_executor_test");
#endif
    try {
        test_json();
        test_eval();
    } catch (const std::runtime_error& e) {
        lerr() << e.what();
        return 1;
    }
    return 0;
}
