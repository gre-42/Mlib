#pragma once
#include <Mlib/Throw_Or_Abort.hpp>
#include <map>

namespace Mlib {

template <class Func>
class GenShaderText {
public:
    GenShaderText(const Func& func)
        : func_{ func }
    {}
    template <class... Args>
    const char* operator() (const Args... args) {
        static std::map<std::tuple<Args...>, std::pair<std::string, const char*>> texts;
        auto key = std::tuple(args...);
        if (auto it = texts.find(key); it != texts.end()) {
            return it->second.second;
        }
        std::string text = func_(args...);
        if (!texts.insert(std::make_pair(key, std::make_pair(std::move(text), text.c_str()))).second) {
            THROW_OR_ABORT("Could not insert shader text");
        }
        return texts.at(key).second;
    }
private:
    Func func_;
};

}
