#include <Mlib/Env.hpp>
#include <Mlib/Macro_Executor/Macro_Line_Executor.hpp>
#include <Mlib/Macro_Executor/Macro_Recorder.hpp>
#include <Mlib/Macro_Executor/Notifying_Json_Macro_Arguments.hpp>
#include <iostream>

using namespace Mlib;

void test_scn() {
    MacroLineExecutor::JsonUserFunction json_user_function = [](
        const std::string& context,
        const MacroLineExecutor& macro_line_executor,
        const std::string& name,
        const JsonMacroArguments& arguments)
    {
        return true;
    };
    std::list<std::string> search_path{"."};
    NotifyingJsonMacroArguments global_substitutions;
    MacroRecorder mr;
    MacroLineExecutor mle{
        mr,
        "script.scn",
        &search_path,
        json_user_function,
        "context",
        global_substitutions,
        true};
    mr(mle);
}

void test_json() {
    MacroLineExecutor::JsonUserFunction json_user_function = [](
        const std::string& context,
        const MacroLineExecutor& macro_line_executor,
        const std::string& name,
        const JsonMacroArguments& arguments)
    {
        linfo() << "Name: " << name;
        linfo() << "Arguments: " << arguments;
        return true;
    };
    std::list<std::string> search_path{"."};
    NotifyingJsonMacroArguments global_substitutions;
    MacroRecorder mr;
    MacroLineExecutor mle{
        mr,
        "script.scn.json",
        &search_path,
        json_user_function,
        "context",
        global_substitutions,
        true};
    mr(mle);
}

int main(int argc, char** argv) {
    set_app_reldir("macro_executor_test");
    try {
        test_scn();
        test_json();
    } catch (const std::runtime_error& e) {
        std::cerr << e.what() << std::endl;
        return 1;
    }
    return 0;
}
