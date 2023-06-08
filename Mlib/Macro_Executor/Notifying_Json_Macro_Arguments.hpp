#pragma once
#include <Mlib/Macro_Executor/Json_Macro_Arguments.hpp>
#include <mutex>

namespace Mlib {

class JsonMacroArgumentsAndLock;

class NotifyingJsonMacroArguments {
    friend JsonMacroArgumentsAndLock;
public:
    NotifyingJsonMacroArguments();
    void set_and_notify(const std::string& key, const nlohmann::json& value);
    void merge_and_notify(const JsonMacroArguments& other);
    template <class TResult>
    TResult at(const std::string& key) const {
        return json_macro_arguments_.at<TResult>(key);
    }
    JsonMacroArgumentsAndLock json_macro_arguments() const;
    void add_observer(const std::function<void()>& func);
    void clear_observers();
private:
    JsonMacroArguments json_macro_arguments_;
    std::list<std::function<void()>> observers_;
    mutable std::recursive_mutex mutex_;
};

class JsonMacroArgumentsAndLock {
public:
    explicit JsonMacroArgumentsAndLock(const NotifyingJsonMacroArguments& args);
    operator const JsonMacroArguments&() const;
    operator const nlohmann::json&() const;
    void unlock();
private:
    std::unique_lock<std::recursive_mutex> lock_;
    const NotifyingJsonMacroArguments& args_;
};

class JsonMacroArgumentsObserverGuard {
public:
    JsonMacroArgumentsObserverGuard(NotifyingJsonMacroArguments& nma);
    ~JsonMacroArgumentsObserverGuard();
private:
    NotifyingJsonMacroArguments& nma_;
};

}
