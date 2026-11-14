#pragma once
#include <optional>
#include <string>

namespace Mlib {

#ifdef MALLOC_WRAPPING_ENABLED

void enable_wrap_malloc();
bool wrap_malloc_enabled();

class MallocGuard {
public:
    explicit MallocGuard(std::string name);
    ~MallocGuard();
private:
    std::string name_;
    const std::string* parent_;
};

class StackGuard {
public:
    StackGuard();
    ~StackGuard();
private:
    const std::string* tl_;
};

void print_allocated();

#define STACK_GUARD(var) StackGuard var
#define MALLOC_GUARD(var, name) std::optional<MallocGuard> var; if (wrap_malloc_enabled()) var.emplace(name)
#define PRINT_ALLOCATED() if (wrap_malloc_enabled()) print_allocated()
#else
#define STACK_GUARD(var)
#define MALLOC_GUARD(var, name)
#define PRINT_ALLOCATED()
#endif

}
