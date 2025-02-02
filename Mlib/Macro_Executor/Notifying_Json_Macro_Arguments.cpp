#include "Notifying_Json_Macro_Arguments.hpp"
#include <mutex>

using namespace Mlib;

NotifyingJsonMacroArguments::NotifyingJsonMacroArguments() = default;

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

void NotifyingJsonMacroArguments::add_observer(std::function<void()> func) {
    std::scoped_lock lock{ mutex_ };
    observers_.emplace_back(std::move(func));
}

void NotifyingJsonMacroArguments::clear_observers() {
    std::scoped_lock lock{ mutex_ };
    observers_.clear();
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

JsonMacroArgumentsObserverGuard::JsonMacroArgumentsObserverGuard(NotifyingJsonMacroArguments& nma)
: nma_{nma}
{}

JsonMacroArgumentsObserverGuard::~JsonMacroArgumentsObserverGuard() {
    nma_.clear_observers();
}
