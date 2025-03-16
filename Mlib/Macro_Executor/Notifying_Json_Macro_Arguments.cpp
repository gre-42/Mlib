#include "Notifying_Json_Macro_Arguments.hpp"
#include <mutex>

using namespace Mlib;

JsonMacroArgumentsObserverToken::JsonMacroArgumentsObserverToken(
    NotifyingJsonMacroArguments& args,
    std::list<std::function<void()>>::iterator it)
    : args_{ args }
    , it_{ std::move(it) }
{}

JsonMacroArgumentsObserverToken::~JsonMacroArgumentsObserverToken() {
    args_.remove_observer(it_);
}

NotifyingJsonMacroArguments::NotifyingJsonMacroArguments() = default;

NotifyingJsonMacroArguments::~NotifyingJsonMacroArguments() {
    if (!observers_.empty()) {
        verbose_abort("NotifyingJsonMacroArguments: Observers remain in dtor");
    }
}

void NotifyingJsonMacroArguments::set_and_notify(const std::string& key, const nlohmann::json& value) {
    std::scoped_lock lock{ mutex_ };
    json_macro_arguments_.set(key, value);
    for (const auto& f : observers_) {
        f();
    }
}

void NotifyingJsonMacroArguments::merge_and_notify(const JsonMacroArguments& other) {
    std::scoped_lock lock{ mutex_ };
    json_macro_arguments_.merge(other);
    for (const auto& f : observers_) {
        f();
    }
}

JsonMacroArgumentsAndLock NotifyingJsonMacroArguments::json_macro_arguments() const {
    return JsonMacroArgumentsAndLock{*this};
}

JsonMacroArgumentsObserverToken NotifyingJsonMacroArguments::add_observer(
    std::function<void()> func)
{
    std::scoped_lock lock{ mutex_ };
    return { *this, observers_.emplace(observers_.end(), std::move(func)) };
}

void NotifyingJsonMacroArguments::remove_observer(
    const std::list<std::function<void()>>::iterator& it)
{
    std::scoped_lock lock{ mutex_ };
    observers_.erase(it);
}

JsonMacroArgumentsAndLock::JsonMacroArgumentsAndLock(const NotifyingJsonMacroArguments& args)
    : lock_{ args.mutex_ }
    , args_{ args }
{}

JsonMacroArgumentsAndLock::operator const JsonMacroArguments&() const {
    if (!lock_.owns_lock()) {
        THROW_OR_ABORT("JsonMacroArgumentsAndLock not locked");
    }
    return args_.json_macro_arguments_;
}

JsonMacroArgumentsAndLock::operator const nlohmann::json&() const {
    if (!lock_.owns_lock()) {
        THROW_OR_ABORT("JsonMacroArgumentsAndLock not locked");
    }
    return args_.json_macro_arguments_.json();
}

void JsonMacroArgumentsAndLock::unlock() {
    lock_.unlock();
}
