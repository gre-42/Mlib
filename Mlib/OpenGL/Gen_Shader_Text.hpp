#pragma once
#include <Mlib/Os/Env.hpp>
#include <Mlib/Os/Os.hpp>
#include <map>
#include <stdexcept>

namespace Mlib {

template <class Func>
class GenShaderText {
public:
    GenShaderText(const Func& func)
        : func_{ func }
        , check_{ getenv_default_bool("CHECK_SHADER_TEXTS", false) }
    {}
    template <class... Args>
    const char* operator() (const Args... args) {
        static std::map<std::tuple<Args...>, std::pair<std::string, const char*>> texts;
        auto key = std::tuple(args...);
        if (auto it = texts.find(key); it != texts.end()) {
            if (check_) {
                std::string text = func_(args...);
                if (text != it->second.first) {
                    lerr() << "===== New =====";
                    lerr() << text;
                    lerr() << "===== Old =====";
                    lerr() << it->second.first;
                    throw std::runtime_error("Differing shader texts");
                }
            }
            return it->second.second;
        }
        {
            std::string text = func_(args...);
            auto res = texts.try_emplace(key, std::move(text), text.c_str());
            if (!res.second) {
                throw std::runtime_error("Could not insert shader text");
            }
            return res.first->second.second;
        }
    }
private:
    Func func_;
    bool check_;
};

}
