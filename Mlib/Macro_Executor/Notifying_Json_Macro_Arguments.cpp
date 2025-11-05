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

void NotifyingJsonMacroArguments::set_and_notify(const std::string_view& key, const nlohmann::json& value) {
    set_and_notify_generic(key, value);
}

void NotifyingJsonMacroArguments::set_and_notify(const std::vector<std::string>& key, const nlohmann::json& value) {
    set_and_notify_generic(key, value);
}

template <JsonKey Key>
void NotifyingJsonMacroArguments::set_and_notify_generic(const Key& key, const nlohmann::json& value) {
    ++notification_counter_;
    {
        DestructionGuard dg([&](){
            --notification_counter_;
        });
        {
            std::scoped_lock lock{ json_mutex_ };
            json_macro_arguments_.set(key, value);
        }
        std::shared_lock lock{ observer_mutex_ };
        for (const auto& f : observers_) {
            f();
        }
    }
    if (notification_counter_ == 0) {
        std::shared_lock lock{ observer_mutex_ };
        for (const auto& f : finalizers_) {
            f();
        }
    }
}

void NotifyingJsonMacroArguments::merge_and_notify(const JsonView& other) {
    ++notification_counter_;
    {
        DestructionGuard dg([&](){
            --notification_counter_;
        });
        {
            std::scoped_lock lock{ json_mutex_ };
            json_macro_arguments_.merge(other);
        }
        std::shared_lock lock{ observer_mutex_ };
        for (const auto& f : observers_) {
            f();
        }
    }
    if (notification_counter_ == 0) {
        std::shared_lock lock{ observer_mutex_ };
        for (const auto& f : finalizers_) {
            f();
        }
    }
}

void NotifyingJsonMacroArguments::assign_and_notify(const JsonView& other) {
    ++notification_counter_;
    {
        DestructionGuard dg([&](){
            --notification_counter_;
        });
        {
            std::scoped_lock lock{ json_mutex_ };
            json_macro_arguments_.clear();
            json_macro_arguments_.merge(other);
        }
        std::shared_lock lock{ observer_mutex_ };
        for (const auto& f : observers_) {
            f();
        }
    }
    if (notification_counter_ == 0) {
        std::shared_lock lock{ observer_mutex_ };
        for (const auto& f : finalizers_) {
            f();
        }
    }
}

bool NotifyingJsonMacroArguments::contains(const std::string_view& key) const {
    std::shared_lock lock{json_mutex_};
    return json_macro_arguments_.contains(key);
}

bool NotifyingJsonMacroArguments::contains(const std::vector<std::string>& key) const {
    std::shared_lock lock{json_mutex_};
    return json_macro_arguments_.contains(key);
}

JsonMacroArgumentsAndLock NotifyingJsonMacroArguments::json_macro_arguments() const {
    return JsonMacroArgumentsAndLock{*this};
}

WritableJsonMacroArgumentsAndLock NotifyingJsonMacroArguments::writable_json_macro_arguments() {
    return WritableJsonMacroArgumentsAndLock{*this};
}

JsonMacroArgumentsObserverToken NotifyingJsonMacroArguments::add_observer(
    std::function<void()> func)
{
    std::scoped_lock lock{ observer_mutex_ };
    return JsonMacroArgumentsObserverToken{
        [this, it=observers_.emplace(observers_.end(), std::move(func))](){
            remove_observer(it);
        }
    };
}

JsonMacroArgumentsObserverToken NotifyingJsonMacroArguments::add_finalizer(
    std::function<void()> func)
{
    std::scoped_lock lock{ observer_mutex_ };
    return JsonMacroArgumentsObserverToken{
        [this, it=finalizers_.emplace(finalizers_.end(), std::move(func))](){
            remove_finalizer(it);
        }
    };
}

void NotifyingJsonMacroArguments::remove_observer(
    const std::list<std::function<void()>>::iterator& it)
{
    std::scoped_lock lock{ observer_mutex_ };
    observers_.erase(it);
}

void NotifyingJsonMacroArguments::remove_finalizer(
    const std::list<std::function<void()>>::iterator& it)
{
    std::scoped_lock lock{ observer_mutex_ };
    finalizers_.erase(it);
}

// Readonly

JsonMacroArgumentsAndLock::JsonMacroArgumentsAndLock(const NotifyingJsonMacroArguments& args)
    : lock_{ args.json_mutex_ }
    , args_{ args }
{}

JsonMacroArgumentsAndLock::~JsonMacroArgumentsAndLock() = default;

const JsonMacroArguments* JsonMacroArgumentsAndLock::operator -> () const {
    if (!lock_.owns_lock()) {
        THROW_OR_ABORT("JsonMacroArgumentsAndLock not locked");
    }
    return &args_.json_macro_arguments_;
}

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

// Writable

WritableJsonMacroArgumentsAndLock::WritableJsonMacroArgumentsAndLock(NotifyingJsonMacroArguments& args)
    : lock_{ args.json_mutex_ }
    , args_{ args }
{
    ++args.notification_counter_;
}

WritableJsonMacroArgumentsAndLock::~WritableJsonMacroArgumentsAndLock() {
    if (lock_.owns_lock()) {
        --args_.notification_counter_;
        lwarn() << "WritableJsonMacroArgumentsAndLock::unlock not called";
    }
}

JsonMacroArguments* WritableJsonMacroArgumentsAndLock::operator -> () {
    if (!lock_.owns_lock()) {
        THROW_OR_ABORT("WritableJsonMacroArgumentsAndLock not locked");
    }
    return &args_.json_macro_arguments_;
}

WritableJsonMacroArgumentsAndLock::operator JsonMacroArguments&() {
    if (!lock_.owns_lock()) {
        THROW_OR_ABORT("WritableJsonMacroArgumentsAndLock not locked");
    }
    return args_.json_macro_arguments_;
}

void WritableJsonMacroArgumentsAndLock::unlock_and_notify() {
    if (!lock_.owns_lock()) {
        THROW_OR_ABORT("WritableJsonMacroArgumentsAndLock not locked");
    }
    lock_.unlock();
    {
        DestructionGuard dg([&](){
            --args_.notification_counter_;
        });
        std::shared_lock lock{ args_.observer_mutex_ };
        for (const auto& f : args_.observers_) {
            f();
        }
    }
    if (args_.notification_counter_ == 0) {
        std::shared_lock lock{ args_.observer_mutex_ };
        for (const auto& f : args_.finalizers_) {
            f();
        }
    }
}
