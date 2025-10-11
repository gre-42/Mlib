#include "Destruction_Functions.hpp"
#include <Mlib/Memory/Recursive_Deletion.hpp>
#include <Mlib/Os/Os.hpp>
#include <mutex>

using namespace Mlib;

DestructionFunctions::DestructionFunctions()
    : clearing_{ false }
{}

DestructionFunctions::~DestructionFunctions() {
    if (!funcs_.empty()) {
        print_source_locations();
        verbose_abort("Destruction functions remain");
    }
}

void DestructionFunctions::print_source_locations() const {
    for (const auto& [_, funcs] : funcs_) {
        for (const auto& e : funcs) {
            lerr() << e.loc;
        }
    }
}

void DestructionFunctions::add(
    DestructionFunctionsRemovalTokens& tokens,
    std::function<void()> f,
    SourceLocation loc)
{
    if (clearing_) {
        verbose_abort("DestructionFunctions::add called during clearing");
    }
    std::scoped_lock lock{ mutex_ };
    funcs_[&tokens].emplace_back(std::move(f), loc);
}

void DestructionFunctions::remove(DestructionFunctionsRemovalTokens& tokens) {
    // Erase token and ignore result. Destruction order in
    // DestructionFunctions::clear is arbitrary and the token
    // can therefore already have been deleted.
    if (clearing_) {
        funcs_.erase(&tokens);
    } else {
        std::scoped_lock lock{ mutex_ };
        funcs_.erase(&tokens);
        // if (funcs_.erase(&tokens) != 1) {
        //     verbose_abort("Could not erase destruction removal token");
        // }
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
        clear_list_recursively_with_lock(node.mapped(), lock, [](auto& e) {
            // lerr() << e.loc;
            e.func();
        });
    });
    clearing_ = false;
}

bool DestructionFunctions::empty() const {
    std::scoped_lock lock{ mutex_ };
    return funcs_.empty();
}

DestructionFunctionsRemovalTokens::DestructionFunctionsRemovalTokens(
    DestructionFunctions& funcs,
    SourceLocation loc)
    : DestructionFunctionsRemovalTokens{ &funcs, loc }
{}

DestructionFunctionsRemovalTokens::DestructionFunctionsRemovalTokens(
    DestructionFunctions* funcs,
    SourceLocation loc)
    : loc_{ loc }
    , funcs_{ nullptr }
{
    set(funcs, loc);
}

DestructionFunctionsRemovalTokens::~DestructionFunctionsRemovalTokens() {
    clear_unsafe();
}

void DestructionFunctionsRemovalTokens::add(std::function<void()> f, SourceLocation loc) {
    std::scoped_lock lock{ mutex_ };
    if (funcs_ == nullptr) {
        verbose_abort("DestructionFunctionsRemovalTokens::add on destroyed functions");
    }
    funcs_->add(*this, std::move(f), std::move(loc));
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

void DestructionFunctionsRemovalTokens::set(DestructionFunctions& funcs, SourceLocation loc) {
    set(&funcs, loc);
}

void DestructionFunctionsRemovalTokens::set(DestructionFunctions* funcs, SourceLocation loc) {
    std::scoped_lock lock{ mutex_ };
    clear_unsafe();
    loc_ = loc;
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
