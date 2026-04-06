#pragma once

namespace Mlib {

class IContext;

class ContextQuery {
public:
    static void set_context(IContext& context);
    static void clear_context();
    static bool is_initialized();
private:
    static IContext& context();
    static IContext* context_;
};

class ContextQueryGuard {
public:
    explicit ContextQueryGuard(IContext& context);
    ~ContextQueryGuard();
};

}
