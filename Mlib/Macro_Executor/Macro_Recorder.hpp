#pragma once
#include <Mlib/Json/Misc.hpp>
#include <map>
#include <string>

namespace Mlib {

class MacroLineExecutor;
class JsonMacroArguments;

struct JsonMacro {
    std::string filename;
    nlohmann::json content;
};

class MacroRecorder {
    friend MacroLineExecutor;
public:
    MacroRecorder();
    ~MacroRecorder();
    void operator () (
        const MacroLineExecutor& macro_line_executor,
        const JsonMacroArguments* caller_args);
private:
    std::map<std::string, JsonMacro> json_macros_;
};

}
