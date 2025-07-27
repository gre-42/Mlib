#include "Notifying_Json_Macro_Arguments.hpp"
#include <Mlib/Memory/Destruction_Guard.hpp>
#include <mutex>

using namespace Mlib;

JsonMacroArgumentsObserverToken::JsonMacroArgumentsObserverToken(
    std::function<void()> cleanup)
    : cleanup_{ std::move(cleanup) }
{}

JsonMacroArgumentsObserverToken::~JsonMacroArgumentsObserverToken() {
    cleanup_();
}

NotifyingJsonMacroArguments::NotifyingJsonMacroArguments()
    : notification_counter_{ 0 }
{}

NotifyingJsonMacroArguments::~NotifyingJsonMacroArguments() {
    if (!observers_.empty()) {
        verbose_abort("NotifyingJsonMacroArguments: Observers remain in dtor");
    }
}

void NotifyingJsonMacroArguments::set_and_notify(const std::string& key, const nlohmann::json& value) {
    std::shared_lock lock{ mutex_ };
    ++notification_counter_;
    {
        DestructionGuard dg([&](){
            --notification_counter_;
        });
        json_macro_arguments_.set(key, value);
        for (const auto& f : observers_) {
            f();
        }
    }
    if (notification_counter_ == 0) {
        for (const auto& f : finalizers_) {
            f();
        }
    }
}

void NotifyingJsonMacroArguments::merge_and_notify(const JsonMacroArguments& other) {
    std::shared_lock lock{ mutex_ };
    ++notification_counter_;
    {
        DestructionGuard dg([&](){
            --notification_counter_;
        });
        json_macro_arguments_.merge(other);
        for (const auto& f : observers_) {
            f();
        }
    }
    if (notification_counter_ == 0) {
        for (const auto& f : finalizers_) {
            f();
        }
    }
}

JsonMacroArgumentsAndLock NotifyingJsonMacroArguments::json_macro_arguments() const {
    return JsonMacroArgumentsAndLock{*this};
}

JsonMacroArgumentsObserverToken NotifyingJsonMacroArguments::add_observer(
    std::function<void()> func)
{
    std::scoped_lock lock{ mutex_ };
    return JsonMacroArgumentsObserverToken{
        [this, it=observers_.emplace(observers_.end(), std::move(func))](){
            remove_observer(it);
        }
    };
}

JsonMacroArgumentsObserverToken NotifyingJsonMacroArguments::add_finalizer(
    std::function<void()> func)
{
    std::scoped_lock lock{ mutex_ };
    return JsonMacroArgumentsObserverToken{
        [this, it=finalizers_.emplace(finalizers_.end(), std::move(func))](){
            remove_finalizer(it);
        }
    };
}

void NotifyingJsonMacroArguments::remove_observer(
    const std::list<std::function<void()>>::iterator& it)
{
    std::scoped_lock lock{ mutex_ };
    observers_.erase(it);
}

void NotifyingJsonMacroArguments::remove_finalizer(
    const std::list<std::function<void()>>::iterator& it)
{
    std::scoped_lock lock{ mutex_ };
    finalizers_.erase(it);
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
