#pragma once
#include <Mlib/Object.hpp>
#include <list>
#include <memory>

namespace Mlib {

class SharedPtrs {
public:
    SharedPtrs();
    ~SharedPtrs();
    void add(std::shared_ptr<Object> ptr);
    void clear();
private:
    std::list<std::shared_ptr<Object>> ptrs_;
};

};
