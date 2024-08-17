#pragma once
#include <Mlib/Memory/Destruction_Functions.hpp>
#include <functional>
#include <list>

namespace Mlib {

class DestructionFunctionsTokensList {
public:
    DestructionFunctionsTokensList();
    ~DestructionFunctionsTokensList();
    void add(DestructionFunctions& funcs, std::function<void()> f, SourceLocation loc);
    void clear();
private:
    std::list<DestructionFunctionsRemovalTokens> removal_tokens_;
};

}
