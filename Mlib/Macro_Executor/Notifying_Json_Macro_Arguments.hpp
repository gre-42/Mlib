#pragma once
#include <Mlib/Macro_Executor/Json_Macro_Arguments.hpp>
#include <Mlib/Threads/Recursive_Shared_Mutex.hpp>

namespace Mlib {

class JsonMacroArgumentsAndLock;

class NotifyingJsonMacroArguments;

class JsonMacroArgumentsObserverToken {
    friend NotifyingJsonMacroArguments;
    JsonMacroArgumentsObserverToken(const JsonMacroArgumentsObserverToken&) = delete;
    JsonMacroArgumentsObserverToken& operator = (const JsonMacroArgumentsObserverToken&) = delete;
private:
    inline JsonMacroArgumentsObserverToken(
        NotifyingJsonMacroArguments& args,
        std::list<std::function<void()>>::iterator it);
public:
    ~JsonMacroArgumentsObserverToken();
private:
    NotifyingJsonMacroArguments& args_;
    std::list<std::function<void()>>::iterator it_;
};

class NotifyingJsonMacroArguments {
    friend JsonMacroArgumentsObserverToken;
    friend JsonMacroArgumentsAndLock;
    NotifyingJsonMacroArguments(const NotifyingJsonMacroArguments&) = delete;
    NotifyingJsonMacroArguments& operator = (const NotifyingJsonMacroArguments&) = delete;
public:
    NotifyingJsonMacroArguments();
    ~NotifyingJsonMacroArguments();
    void set_and_notify(const std::string& key, const nlohmann::json& value);
    void merge_and_notify(const JsonMacroArguments& other);
    template <class TResult>
    TResult at(const std::string& key) const {
        std::shared_lock lock{mutex_};
        return json_macro_arguments_.at<TResult>(key);
    }
    template <class TResult>
    TResult at(const std::string& key, const TResult& default_) const {
        std::shared_lock lock{mutex_};
        return json_macro_arguments_.at<TResult>(key, default_);
    }
    JsonMacroArgumentsAndLock json_macro_arguments() const;
    JsonMacroArgumentsObserverToken add_observer(std::function<void()> func);
private:
    void remove_observer(const std::list<std::function<void()>>::iterator& it);
    mutable SafeAtomicRecursiveSharedMutex mutex_;
    JsonMacroArguments json_macro_arguments_;
    std::list<std::function<void()>> observers_;
};

class JsonMacroArgumentsAndLock {
public:
    explicit JsonMacroArgumentsAndLock(const NotifyingJsonMacroArguments& args);
    operator const JsonMacroArguments&() const;
    operator const nlohmann::json&() const;
    void unlock();
private:
    std::shared_lock<SafeAtomicRecursiveSharedMutex> lock_;
    const NotifyingJsonMacroArguments& args_;
};

}
