#pragma once
#include <functional>
#include <iosfwd>
#include <list>

namespace Mlib {

template <class TResourceContext>
class ResourceContextGuard {
public:
    explicit ResourceContextGuard(const TResourceContext& resource_context);
    ~ResourceContextGuard();
};

template <class TResourceContext>
class ResourceContextStack {
    friend ResourceContextGuard<TResourceContext>;
public:
    static TResourceContext primary_resource_context();
    static TResourceContext resource_context();
    static std::function<std::function<void()>(std::function<void()>)>
        generate_thread_runner(
            const TResourceContext& primary_resource_context,
            const TResourceContext& secondary_resource_context);
    static void print_stack(std::ostream& ostr);
protected:
    static std::list<TResourceContext>& resource_context_stack();
};

}
