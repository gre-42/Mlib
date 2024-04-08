#include "Destruction_Functions.hpp"
#include <Mlib/Memory/Recursive_Deletion.hpp>
#include <Mlib/Os/Os.hpp>

using namespace Mlib;

DestructionFunctions::DestructionFunctions()
    : clearing_{ false }
{}

DestructionFunctions::~DestructionFunctions() {
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
        if (funcs_.erase(&tokens) != 1) {
            verbose_abort("Could not erase destruction removal token");
        }
    }
}

void DestructionFunctions::clear() {
    if (clearing_) {
        verbose_abort("DestructionFunctions already clearing");
    }
    std::unique_lock lock{ mutex_ };
    clearing_ = true;
    clear_map_recursively(funcs_, [&lock](auto& node) {
        node.key()->funcs_ = nullptr;
        clear_list_recursively_with_lock(node.mapped(), lock, [](auto& f) { f(); });
    });
    clearing_ = false;
}

bool DestructionFunctions::empty() const {
    std::scoped_lock lock{ mutex_ };
    return funcs_.empty();
}

DestructionFunctionsRemovalTokens::DestructionFunctionsRemovalTokens(DestructionFunctions& funcs)
    : DestructionFunctionsRemovalTokens{ &funcs }
{}

DestructionFunctionsRemovalTokens::DestructionFunctionsRemovalTokens(DestructionFunctions* funcs)
    : funcs_{ nullptr }
{
    set(funcs);
}

DestructionFunctionsRemovalTokens::~DestructionFunctionsRemovalTokens() {
    clear_unsafe();
}

void DestructionFunctionsRemovalTokens::add(std::function<void()> f) {
    std::scoped_lock lock{ mutex_ };
    if (funcs_ == nullptr) {
        verbose_abort("DestructionFunctionsRemovalTokens::add on destroyed functions");
    }
    funcs_->add(*this, std::move(f));
}

void DestructionFunctionsRemovalTokens::clear() {
    std::scoped_lock lock{ mutex_ };
    clear_unsafe();
}

void DestructionFunctionsRemovalTokens::clear_unsafe() {
    if (funcs_ != nullptr) {
        funcs_->remove(*this);
        funcs_ = nullptr;
    }
}

void DestructionFunctionsRemovalTokens::set(DestructionFunctions& funcs) {
    set(&funcs);
}

void DestructionFunctionsRemovalTokens::set(DestructionFunctions* funcs) {
    std::scoped_lock lock{ mutex_ };
    clear_unsafe();
    funcs_ = funcs;
}

bool DestructionFunctionsRemovalTokens::empty() const {
    std::scoped_lock lock{ mutex_ };
    if (funcs_ == nullptr) {
        verbose_abort("DestructionFunctionsRemovalTokens::empty call on null object");
    }
    auto it = funcs_->funcs_.find(const_cast<DestructionFunctionsRemovalTokens*>(this));
    if (it == funcs_->funcs_.end()) {
        verbose_abort("Could not find destruction removal token");
    }
    return it->second.empty();
}

bool DestructionFunctionsRemovalTokens::is_null() const {
    std::scoped_lock lock{ mutex_ };
    return funcs_ == nullptr;
}
