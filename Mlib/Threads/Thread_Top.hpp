#pragma once
#include <compare>
#include <string>
#include <thread>

namespace Mlib {

struct ThreadIdentifier {
    std::string name;
    std::thread::id id;
    std::strong_ordering operator <=> (const ThreadIdentifier&) const = default;
};

class FunctionGuard {
public:
    explicit FunctionGuard(std::string task_name);
    ~FunctionGuard();
    void update(std::string task_name);
private:
    ThreadIdentifier id;
    std::string* task_name_;
};

std::string thread_top();

}
