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
    if (!tokens.funcs_it_.has_value()) {
        tokens.funcs_ = this;
        tokens.funcs_it_.emplace(funcs_.insert(funcs_.end(), {tokens}));
    } else if (tokens.funcs_ != this) {
        throw std::runtime_error("DestructionFunctions::add mismatch");
    }
    (*tokens.funcs_it_)->funcs.emplace_back(std::move(f), loc);
}

void DestructionFunctions::remove(DestructionFunctionsRemovalTokens& tokens) {
    // Erase token and ignore result. Destruction order in
    // DestructionFunctions::clear is arbitrary and the token
    // can therefore already have been deleted.
    if (clearing_) {
        if (tokens.funcs_it_.has_value()) {
            funcs_.erase(*tokens.funcs_it_);
            tokens.funcs_ = nullptr;
            tokens.funcs_it_.reset();
        }
    } else {
        std::scoped_lock lock{ mutex_ };
        if (tokens.funcs_it_.has_value()) {
            funcs_.erase(*tokens.funcs_it_);
            tokens.funcs_ = nullptr;
            tokens.funcs_it_.reset();
        }
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
    clear_list_recursively(funcs_, [&lock](auto& node) {
        node.tokens.funcs_ = nullptr;
        node.tokens.funcs_it_.reset();
        clear_list_recursively_with_lock(node.funcs, lock, [](auto& e) {
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
        funcs_it_.reset();
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
    return funcs_it_.value()->funcs.empty();
}

bool DestructionFunctionsRemovalTokens::is_null() const {
    std::scoped_lock lock{ mutex_ };
    return funcs_ == nullptr;
}

EarlyAndLateDestructionFunctionsRemovalTokens::EarlyAndLateDestructionFunctionsRemovalTokens(
    EarlyAndLateDestructionFunctions& funcs,
    SourceLocation loc)
    : deflt{funcs.deflt, loc}
    , early{funcs.early, loc}
    , late{funcs.late, loc}
{}

EarlyAndLateDestructionFunctionsRemovalTokens::EarlyAndLateDestructionFunctionsRemovalTokens(
    EarlyAndLateDestructionFunctions* funcs,
    SourceLocation loc)
    : deflt{funcs ? &funcs->deflt : nullptr, loc}
    , early{funcs ? &funcs->early : nullptr, loc}
    , late{funcs ? &funcs->late : nullptr, loc}
{}

void EarlyAndLateDestructionFunctionsRemovalTokens::set(
    EarlyAndLateDestructionFunctions& funcs,
    SourceLocation loc)
{
    deflt.set(funcs.deflt, loc);
    early.set(funcs.early, loc);
    late.set(funcs.late, loc);
}

void EarlyAndLateDestructionFunctionsRemovalTokens::set(EarlyAndLateDestructionFunctions* funcs, SourceLocation loc) {
    deflt.set(funcs ? &funcs->deflt : nullptr, loc);
    early.set(funcs ? &funcs->early : nullptr, loc);
    late.set(funcs ? &funcs->late : nullptr, loc);
}

void EarlyAndLateDestructionFunctions::clear() {
    deflt.clear();
    early.clear();
    late.clear();
}

bool EarlyAndLateDestructionFunctions::empty() const {
    return deflt.empty() && early.empty() && late.empty();
}

void EarlyAndLateDestructionFunctions::print_source_locations() const {
    lerr() << "Default";
    deflt.print_source_locations();
    lerr() << "Early";
    early.print_source_locations();
    lerr() << "Late";
    late.print_source_locations();
}
