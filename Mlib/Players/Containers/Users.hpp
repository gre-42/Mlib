#pragma once
#include <Mlib/Memory/Dangling_Base_Class.hpp>
#include <cstdint>
#include <functional>

namespace Mlib {

class Users: public virtual DanglingBaseClass {
public:
    Users();
    ~Users();
    uint32_t get_user_count() const;
    void set_user_count(uint32_t user_count);
    bool for_each_user(const std::function<bool(uint32_t)>& operation);
private:
    uint32_t user_count_;
};

}
