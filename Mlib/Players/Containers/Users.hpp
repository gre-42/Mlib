#pragma once
#include <cstdint>
#include <functional>

namespace Mlib {

class Users {
public:
    Users();
    ~Users();
    void set_user_count(uint32_t user_count);
    void for_each_user(const std::function<void(uint32_t)>& operation);
private:
    uint32_t user_count_;
};

}
