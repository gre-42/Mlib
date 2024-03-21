#pragma once
#include <atomic>
#include <functional>
#include <list>
#include <map>
#include <mutex>

namespace Mlib {

class DestructionFunctions;

class DestructionFunctionsRemovalTokens {
    friend DestructionFunctions;
    DestructionFunctionsRemovalTokens(const DestructionFunctionsRemovalTokens&) = delete;
    DestructionFunctionsRemovalTokens& operator = (const DestructionFunctionsRemovalTokens&) = delete;
public:
    explicit DestructionFunctionsRemovalTokens(DestructionFunctions& funcs);
    ~DestructionFunctionsRemovalTokens();
    void add(std::function<void()> f);
    void clear();
private:
    DestructionFunctions* funcs_;
};

class DestructionFunctions {
    friend DestructionFunctionsRemovalTokens;
    DestructionFunctions(const DestructionFunctions&) = delete;
    DestructionFunctions& operator = (const DestructionFunctions&) = delete;
    using Funcs = std::map<DestructionFunctionsRemovalTokens*, std::list<std::function<void()>>>;
private:
    void add(DestructionFunctionsRemovalTokens& tokens, std::function<void()> f);
    void remove(DestructionFunctionsRemovalTokens& tokens);
    Funcs funcs_;
    std::mutex mutex_;
    std::atomic_bool clearing_;
public:
    DestructionFunctions();
    ~DestructionFunctions();
    DestructionFunctionsRemovalTokens forever;
    void clear();
};

}
