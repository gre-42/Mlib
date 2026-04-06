#pragma once
#include <Mlib/Misc/Source_Location.hpp>
#include <Mlib/Threads/Fast_Mutex.hpp>
#include <atomic>
#include <functional>
#include <list>
#include <optional>

namespace Mlib {

class DestructionFunctionsRemovalTokens;

struct FuncAndSourceLocation {
    std::function<void()> func;
    SourceLocation loc;
};

struct TokensAndFuncs {
    DestructionFunctionsRemovalTokens& tokens;
    std::list<FuncAndSourceLocation> funcs;
};

class DestructionFunctions {
    friend DestructionFunctionsRemovalTokens;
    DestructionFunctions(const DestructionFunctions&) = delete;
    DestructionFunctions& operator = (const DestructionFunctions&) = delete;
    using Funcs = std::list<TokensAndFuncs>;
private:
    void add(
        DestructionFunctionsRemovalTokens& tokens,
        std::function<void()> f,
        SourceLocation loc);
    void remove(DestructionFunctionsRemovalTokens& tokens);
    Funcs funcs_;
    mutable FastMutex mutex_;
    std::atomic_bool clearing_;
public:
    DestructionFunctions();
    ~DestructionFunctions();
    void clear();
    bool empty() const;
    void print_source_locations() const;
};

class DestructionFunctionsRemovalTokens {
    friend DestructionFunctions;
    DestructionFunctionsRemovalTokens(const DestructionFunctionsRemovalTokens&) = delete;
    DestructionFunctionsRemovalTokens& operator = (const DestructionFunctionsRemovalTokens&) = delete;
public:
    DestructionFunctionsRemovalTokens(DestructionFunctions& funcs, SourceLocation loc);
    DestructionFunctionsRemovalTokens(DestructionFunctions* funcs, SourceLocation loc);
    ~DestructionFunctionsRemovalTokens();
    void add(std::function<void()> f, SourceLocation loc);
    void clear();
    void set(DestructionFunctions& funcs, SourceLocation loc);
    void set(DestructionFunctions* funcs, SourceLocation loc);
    bool empty() const;
    bool is_null() const;
    inline const SourceLocation loc() const {
        return loc_;
    }
private:
    void clear_unsafe();
    mutable FastMutex mutex_;
    SourceLocation loc_;
    DestructionFunctions* funcs_;
    std::optional<DestructionFunctions::Funcs::iterator> funcs_it_;
};

struct EarlyAndLateDestructionFunctions;

struct EarlyAndLateDestructionFunctionsRemovalTokens {
    DestructionFunctionsRemovalTokens deflt;
    DestructionFunctionsRemovalTokens early;    // depth-first
    DestructionFunctionsRemovalTokens late;     // breadth-first
    EarlyAndLateDestructionFunctionsRemovalTokens(EarlyAndLateDestructionFunctions& funcs, SourceLocation loc);
    EarlyAndLateDestructionFunctionsRemovalTokens(EarlyAndLateDestructionFunctions* funcs, SourceLocation loc);
    void set(EarlyAndLateDestructionFunctions& funcs, SourceLocation loc);
    void set(EarlyAndLateDestructionFunctions* funcs, SourceLocation loc);
};

struct EarlyAndLateDestructionFunctions {
    DestructionFunctions deflt;
    DestructionFunctions early; // depth-first
    DestructionFunctions late;  // breadth-first
    void clear();
    bool empty() const;
    void print_source_locations() const;
};

}
