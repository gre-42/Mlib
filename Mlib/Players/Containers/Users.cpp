#include "Users.hpp"
#include <stdexcept>

using namespace Mlib;

Users::Users()
    : user_count_{ 0 }
{}

Users::~Users() = default;

uint32_t Users::get_user_count() const {
    return user_count_;
}

void Users::set_user_count(uint32_t user_count) {
    // if (user_count == 0) {
    //     throw std::runtime_error("User count cannot be zero");
    // }
    user_count_ = std::move(user_count);
}

bool Users::for_each_user(const std::function<bool(uint32_t)>& operation) {
    // if (user_count_ == 0) {
    //     throw std::runtime_error("User count not set");
    // }
    for (uint32_t i = 0; i < user_count_; ++i) {
        if (!operation(i)) {
            return false;
        }
    }
    return true;
}
