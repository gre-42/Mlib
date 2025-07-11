#pragma once
#include <Mlib/Macro_Executor/Macro_Line_Executor.hpp>
#include <Mlib/Macro_Executor/Notifying_Json_Macro_Arguments.hpp>
#include <Mlib/Threads/Fast_Mutex.hpp>

namespace Mlib {

class ExpressionWatcher {
public:
    inline ExpressionWatcher(MacroLineExecutor mle)
        : mle_{ std::move(mle) }
        , result_may_have_changed_{ true }
        , ot_{ mle_.add_observer([this](){
            std::scoped_lock lock{ mutex_ };
            result_may_have_changed_ = true;
            }) }
    {}
    ~ExpressionWatcher() = default;
    inline bool result_may_have_changed() const {
        std::scoped_lock lock{ mutex_ };
        auto res = result_may_have_changed_;
        result_may_have_changed_ = false;
        return res;
    }
    inline JsonMacroArgumentsObserverToken add_observer(std::function<void()> func) {
        return mle_.add_observer(std::move(func));
    }
    inline nlohmann::json eval(const std::string& expression) const {
        return mle_.eval(expression);
    }
    inline nlohmann::json eval(const std::string& expression, const JsonView& variables) const {
        return mle_.eval(expression, variables);
    }
    template <class T>
    T eval(const std::string& expression) const {
        return mle_.eval<T>(expression);
    }
    template <class T>
    T eval(const std::string& expression, const JsonView& variables) const {
        return mle_.eval<T>(expression, variables);
    }
    bool eval(const std::vector<std::vector<std::string>>& expression) const {
        return mle_.eval(expression);
    }
    bool eval(const std::vector<std::vector<std::string>>& expression, const JsonView& variables) const {
        return mle_.eval(expression, variables);
    }
    inline void execute(
        const nlohmann::json& j,
        JsonMacroArguments* local_json_macro_arguments) const
    {
        mle_(j, local_json_macro_arguments);
    }
private:
    MacroLineExecutor mle_;
    mutable bool result_may_have_changed_;
    JsonMacroArgumentsObserverToken ot_;
    mutable FastMutex mutex_;
};

}
