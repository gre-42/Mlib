#include "Destruction_Functions.hpp"
#include <Mlib/Memory/Recursive_Deletion.hpp>
#include <Mlib/Os/Os.hpp>

using namespace Mlib;

DestructionFunctions::DestructionFunctions()
    : clearing_{ false }
    , forever{ *this }
{}

DestructionFunctions::~DestructionFunctions() {
    forever.clear();
    // clear();
    if (!funcs_.empty()) {
        verbose_abort("Destruction functions remain");
    }
}

void DestructionFunctions::add(
    DestructionFunctionsRemovalTokens& tokens,
    std::function<void()> f)
{
    if (clearing_) {
        verbose_abort("DestructionFunctions::add called during clearing");
    }
    std::scoped_lock lock{ mutex_ };
    funcs_[&tokens].emplace_back(std::move(f));
}

void DestructionFunctions::remove(DestructionFunctionsRemovalTokens& tokens) {
    if (!clearing_) {
        std::scoped_lock lock{ mutex_ };
        funcs_.erase(&tokens);
    }
}

void DestructionFunctions::clear() {
    if (clearing_) {
        verbose_abort("DestructionFunctions already clearing");
    }
    std::unique_lock lock{ mutex_ };
    clearing_ = true;
    clear_map_recursively(funcs_, [&lock](auto& node) {
        clear_list_recursively_with_lock(node.mapped(), lock, [](auto& f) { f(); });
    });
    clearing_ = false;
}

DestructionFunctionsRemovalTokens::DestructionFunctionsRemovalTokens(DestructionFunctions& funcs)
    : funcs_{ &funcs }
{
    funcs_->add(*this, [this]() { funcs_ = nullptr; });
}

DestructionFunctionsRemovalTokens::~DestructionFunctionsRemovalTokens() {
    clear();
}

void DestructionFunctionsRemovalTokens::add(std::function<void()> f) {
    if (funcs_ == nullptr) {
        verbose_abort("DestructionFunctionsRemovalTokens::add on destroyed functions");
    }
    funcs_->add(*this, std::move(f));
}

void DestructionFunctionsRemovalTokens::clear() {
    if (funcs_ != nullptr) {
        funcs_->remove(*this);
    }
}
