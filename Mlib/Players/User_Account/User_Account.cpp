#include "User_Account.hpp"
#include <Mlib/Json/Json_View.hpp>
#include <Mlib/Macro_Executor/Notifying_Json_Macro_Arguments.hpp>

using namespace Mlib;

UserAccount::UserAccount(
    const MacroLineExecutor& mle,
    std::string name_key)
    : mle_{ mle }
    , name_key_{ std::move(name_key) }
{}

std::string UserAccount::name() const {
    return JsonView{mle_.at(name_key_)}.at<std::string>("name");
}
