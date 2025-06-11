#pragma once
#include <Mlib/Json/Misc.hpp>
#include <map>
#include <mutex>
#include <string>

namespace Mlib {

class MacroLineExecutor;

struct JsonMacro {
    std::string filename;
    nlohmann::json content;
    nlohmann::json block_arguments;
};

class MacroRecorder {
    friend MacroLineExecutor;
public:
    MacroRecorder();
    ~MacroRecorder();
    void operator () (const MacroLineExecutor& macro_line_executor);
private:
    std::map<std::string, JsonMacro> json_macros_;
    std::set<std::string> included_files_;
    std::recursive_mutex include_mutex_;
};

}
