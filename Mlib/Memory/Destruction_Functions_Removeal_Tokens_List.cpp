#include "Destruction_Functions_Removeal_Tokens_List.hpp"
#include <Mlib/Os/Os.hpp>

using namespace Mlib;

DestructionFunctionsTokensList::DestructionFunctionsTokensList() = default;

DestructionFunctionsTokensList::~DestructionFunctionsTokensList() {
    if (!removal_tokens_.empty()) {
        for (const auto& r : removal_tokens_) {
            lerr() << r.loc().file_name() << ':' << r.loc().line();
        }
        verbose_abort("DestructionFunctionsTokensList dtor: list of removal-tokens not empty");
    }
}

void DestructionFunctionsTokensList::add(DestructionFunctions& funcs, std::function<void()> f, SourceLocation loc) {
    auto& t = removal_tokens_.emplace_back(funcs, loc);
    t.add(f, loc);
}

void DestructionFunctionsTokensList::clear() {
    removal_tokens_.clear();
}
