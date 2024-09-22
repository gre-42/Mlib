#pragma once
#include <Mlib/Source_Location.hpp>
#include <Mlib/Threads/Atomic_Mutex.hpp>
#include <atomic>
#include <functional>
#include <list>
#include <map>

namespace Mlib {

class DestructionFunctions;

class DestructionFunctionsRemovalTokens {
    friend DestructionFunctions;
    DestructionFunctionsRemovalTokens(const DestructionFunctionsRemovalTokens&) = delete;
    DestructionFunctionsRemovalTokens& operator = (const DestructionFunctionsRemovalTokens&) = delete;
public:
    explicit DestructionFunctionsRemovalTokens(DestructionFunctions& funcs, SourceLocation loc);
    explicit DestructionFunctionsRemovalTokens(DestructionFunctions* funcs, SourceLocation loc);
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
    mutable AtomicMutex mutex_;
    SourceLocation loc_;
    DestructionFunctions* funcs_;
};

struct FuncAndSourceLocation {
    std::function<void()> func;
    SourceLocation loc;
};

class DestructionFunctions {
    friend DestructionFunctionsRemovalTokens;
    DestructionFunctions(const DestructionFunctions&) = delete;
    DestructionFunctions& operator = (const DestructionFunctions&) = delete;
    using Funcs = std::map<DestructionFunctionsRemovalTokens*, std::list<FuncAndSourceLocation>>;
private:
    void add(
        DestructionFunctionsRemovalTokens& tokens,
        std::function<void()> f,
        SourceLocation loc);
    void remove(DestructionFunctionsRemovalTokens& tokens);
    Funcs funcs_;
    mutable AtomicMutex mutex_;
    std::atomic_bool clearing_;
public:
    DestructionFunctions();
    ~DestructionFunctions();
    void clear();
    bool empty() const;
    void print_source_locations() const;
};

}
