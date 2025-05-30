#include "Users.hpp"
#include <Mlib/Throw_Or_Abort.hpp>

using namespace Mlib;

Users::Users()
    : user_count_{ 0 }
{}

Users::~Users() = default;

void Users::set_user_count(uint32_t user_count) {
    if (user_count_ != 0) {
        THROW_OR_ABORT("User count already set");
    }
    if (user_count == 0) {
        THROW_OR_ABORT("User count cannot be zero");
    }
    user_count_ = std::move(user_count);
}

void Users::for_each_user(const std::function<void(uint32_t)>& operation) {
    if (user_count_ == 0) {
        THROW_OR_ABORT("User count not set");
    }
    for (uint32_t i = 0; i < user_count_; ++i) {
        operation(i);
    }
}
