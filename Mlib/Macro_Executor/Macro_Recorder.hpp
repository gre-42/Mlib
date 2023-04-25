#pragma once
#include <Mlib/Macro_Executor/Json_Macro_Arguments.hpp>
#include <functional>
#include <list>
#include <map>
#include <string>

namespace Mlib {

class MacroLineExecutor;

struct JsonMacro {
    std::string filename;
    nlohmann::json content;
};

struct TextMacro {
    std::string filename;
    std::list<std::string> lines;
};

class MacroRecorder {
    friend MacroLineExecutor;
public:
    void operator () (const MacroLineExecutor& macro_line_executor);
private:
    std::map<std::string, JsonMacro> json_macros_;
    std::map<std::string, TextMacro> text_macros_;
};

}
