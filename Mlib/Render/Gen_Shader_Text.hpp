#pragma once
#include <Mlib/Array/Fixed_Array.hpp>
#include <list>
#include <map>

namespace Mlib {

class Light;

template <class Func>
class GenShaderText {
public:
    GenShaderText(const Func& func)
    : func_{func}
    {}
    template <class... Args>
    const char* operator() (const std::list<std::pair<FixedArray<float, 4, 4>, Light*>>& lights, const Args... args) {
        static std::map<std::tuple<Args...>, std::pair<std::string, const char*>> texts;
        auto key = std::tuple(args...);
        auto it = texts.find(key);
        if (it != texts.end()) {
            return it->second.second;
        }
        std::string text = func_(lights, args...);
        texts.insert(std::make_pair(key, std::make_pair(std::move(text), text.c_str())));
        return texts.at(key).second;
    }
private:
    Func func_;
};

}
