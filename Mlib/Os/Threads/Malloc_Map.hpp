#pragma once
#include <string>

namespace Mlib {

#ifdef MALLOC_WRAPPING_ENABLED
class MallocGuard {
public:
    explicit MallocGuard(std::string name);
    ~MallocGuard();
private:
    std::string name_;
    const std::string* parent_;
};

void print_allocated();

#define MALLOC_GUARD(var, name) MallocGuard var(name)
#define PRINT_ALLOCATED() print_allocated();
#else
#define MALLOC_GUARD(var, name)
#define PRINT_ALLOCATED()
#endif

}
