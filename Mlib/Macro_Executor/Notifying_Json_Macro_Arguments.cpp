#include "Notifying_Json_Macro_Arguments.hpp"
#include <mutex>

using namespace Mlib;

NotifyingJsonMacroArguments::NotifyingJsonMacroArguments() = default;

void NotifyingJsonMacroArguments::set_and_notify(const std::string& key, const nlohmann::json& value) {
    std::scoped_lock lock{mutex_};
    json_macro_arguments_.set(key, value);
    for (const auto& f : observers_) {
        f();
    }
}

void NotifyingJsonMacroArguments::merge_and_notify(const JsonMacroArguments& other) {
    std::scoped_lock lock{mutex_};
    json_macro_arguments_.merge(other);
    for (const auto& f : observers_) {
        f();
    }
}

JsonMacroArguments NotifyingJsonMacroArguments::json_macro_arguments() const {
    std::scoped_lock lock{mutex_};
    return json_macro_arguments_;
}

void NotifyingJsonMacroArguments::add_observer(const std::function<void()>& func) {
    std::scoped_lock lock{mutex_};
    observers_.push_back(func);
}

void NotifyingJsonMacroArguments::clear_observers() {
    std::scoped_lock lock{mutex_};
    observers_.clear();
}

JsonMacroArgumentsObserverGuard::JsonMacroArgumentsObserverGuard(NotifyingJsonMacroArguments& nma)
: nma_{nma}
{}

JsonMacroArgumentsObserverGuard::~JsonMacroArgumentsObserverGuard() {
    nma_.clear_observers();
}
