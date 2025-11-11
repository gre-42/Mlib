#include "User_Account.hpp"
#include <Mlib/Json/Json_View.hpp>
#include <Mlib/Macro_Executor/Notifying_Json_Macro_Arguments.hpp>

using namespace Mlib;

UserAccount::UserAccount(
    const MacroLineExecutor& mle,
    std::string key)
    : mle_{ mle }
    , key_{ std::move(key) }
{}

const std::string& UserAccount::key() const {
    return key_;
}

std::string UserAccount::name() const {
    return JsonView{mle_.at(key_)}.at<std::string>("name");
}
