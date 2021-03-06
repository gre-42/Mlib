#pragma once
#include <map>
#include <stdexcept>
#include <vector>

namespace Mlib {

struct Light;
struct BlendMapTexture;
template <class TDir, class TPos, size_t n>
class TransformationMatrix;

template <class Func>
class GenShaderText {
public:
    GenShaderText(const Func& func)
    : func_{func}
    {}
    template <class... Args>
    const char* operator() (
        const std::vector<std::pair<TransformationMatrix<float, double, 3>, Light*>>& lights,
        const std::vector<BlendMapTexture*>& textures,
        const Args... args) {
        static std::map<std::tuple<Args...>, std::pair<std::string, const char*>> texts;
        auto key = std::tuple(args...);
        auto it = texts.find(key);
        if (it != texts.end()) {
            return it->second.second;
        }
        std::string text = func_(lights, textures, args...);
        if (!texts.insert(std::make_pair(key, std::make_pair(std::move(text), text.c_str()))).second) {
            throw std::runtime_error("Could not insert shader text");
        }
        return texts.at(key).second;
    }
private:
    Func func_;
};

}
