#pragma once
#include <Mlib/Json/Json_Key.hpp>
#include <Mlib/Macro_Executor/Json_Macro_Arguments.hpp>
#include <Mlib/Threads/Recursive_Shared_Mutex.hpp>
#include <atomic>
#include <cstdint>
#include <functional>
#include <string>

namespace Mlib {

class JsonMacroArgumentsAndLock;

class NotifyingJsonMacroArguments;

class JsonMacroArgumentsObserverToken {
    friend NotifyingJsonMacroArguments;
    JsonMacroArgumentsObserverToken(const JsonMacroArgumentsObserverToken&) = delete;
    JsonMacroArgumentsObserverToken& operator = (const JsonMacroArgumentsObserverToken&) = delete;
private:
    inline explicit JsonMacroArgumentsObserverToken(std::function<void()> cleanup);
public:
    ~JsonMacroArgumentsObserverToken();
private:
    std::function<void()> cleanup_;
};

class NotifyingJsonMacroArguments {
    friend JsonMacroArgumentsObserverToken;
    friend JsonMacroArgumentsAndLock;
    NotifyingJsonMacroArguments(const NotifyingJsonMacroArguments&) = delete;
    NotifyingJsonMacroArguments& operator = (const NotifyingJsonMacroArguments&) = delete;
public:
    NotifyingJsonMacroArguments();
    ~NotifyingJsonMacroArguments();
    void set_and_notify(const std::string_view& key, const nlohmann::json& value);
    void set_and_notify(const std::vector<std::string>& key, const nlohmann::json& value);
    void merge_and_notify(const JsonView& other);
    void assign_and_notify(const JsonView& other);
    bool contains(const std::string_view& key) const;
    bool contains(const std::vector<std::string>& key) const;
    inline nlohmann::json at(const std::string_view& key) const {
        std::shared_lock lock{json_mutex_};
        return json_macro_arguments_.at(key);
    }
    template <class TResult>
    TResult at(const std::string_view& key) const {
        std::shared_lock lock{json_mutex_};
        return json_macro_arguments_.at<TResult>(key);
    }
    template <class TResult>
    TResult at(const std::string_view& key, const TResult& default_) const {
        std::shared_lock lock{json_mutex_};
        return json_macro_arguments_.at<TResult>(key, default_);
    }
    JsonMacroArgumentsAndLock json_macro_arguments() const;
    JsonMacroArgumentsObserverToken add_observer(std::function<void()> func);
    JsonMacroArgumentsObserverToken add_finalizer(std::function<void()> func);
private:
    template <JsonKey Key>
    void set_and_notify_generic(const Key& key, const nlohmann::json& value);
    void remove_observer(const std::list<std::function<void()>>::iterator& it);
    void remove_finalizer(const std::list<std::function<void()>>::iterator& it);
    mutable SafeAtomicRecursiveSharedMutex json_mutex_;
    mutable SafeAtomicRecursiveSharedMutex observer_mutex_;
    JsonMacroArguments json_macro_arguments_;
    std::list<std::function<void()>> observers_;
    std::list<std::function<void()>> finalizers_;
    std::atomic_uint32_t notification_counter_;
};

class JsonMacroArgumentsAndLock {
public:
    explicit JsonMacroArgumentsAndLock(const NotifyingJsonMacroArguments& args);
    const JsonMacroArguments* operator -> () const;
    operator const JsonMacroArguments&() const;
    operator const nlohmann::json&() const;
    void unlock();
private:
    std::shared_lock<SafeAtomicRecursiveSharedMutex> lock_;
    const NotifyingJsonMacroArguments& args_;
};

}
