#pragma once
#include <Mlib/Array/Fixed_Array.hpp>
#include <map>
#include <stdexcept>
#include <vector>

namespace Mlib {

class Light;

template <class Func>
class GenShaderText {
public:
    GenShaderText(const Func& func)
    : func_{func}
    {}
    template <class... Args>
    const char* operator() (
        const std::vector<std::pair<TransformationMatrix<float>, Light*>>& lights,
        const std::vector<size_t>& light_noshadow_indices,
        const std::vector<size_t>& light_shadow_indices,
        const std::vector<size_t>& black_shadow_indices,
        const Args... args) {
        static std::map<std::tuple<Args...>, std::pair<std::string, const char*>> texts;
        auto key = std::tuple(args...);
        auto it = texts.find(key);
        if (it != texts.end()) {
            return it->second.second;
        }
        std::string text = func_(lights, light_noshadow_indices, light_shadow_indices, black_shadow_indices, args...);
        if (!texts.insert(std::make_pair(key, std::make_pair(std::move(text), text.c_str()))).second) {
            throw std::runtime_error("Could not insert shader text");
        }
        return texts.at(key).second;
    }
private:
    Func func_;
};

}
