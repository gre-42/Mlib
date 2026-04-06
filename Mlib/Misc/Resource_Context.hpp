#pragma once
#include <Mlib/Threads/Thread_Local.hpp>
#include <functional>
#include <iosfwd>
#include <list>

namespace Mlib {

template <class TResourceContext>
class ResourceContextGuard {
    ResourceContextGuard(const ResourceContextGuard &) = delete;
    ResourceContextGuard &operator=(const ResourceContextGuard &) = delete;

public:
    explicit ResourceContextGuard(TResourceContext &resource_context);
    ~ResourceContextGuard();

private:
    TResourceContext resource_context_;
    TResourceContext *old_primary_resource_context_;
    TResourceContext *old_secondary_resource_context_;
};

template <class TResourceContext>
class ResourceContextStack {
    friend ResourceContextGuard<TResourceContext>;
    ResourceContextStack() = delete;

public:
    static TResourceContext& primary_resource_context();
    static TResourceContext& resource_context();
    static std::function<std::function<void()>(std::function<void()>)>
        generate_thread_runner(
            TResourceContext& primary_resource_context,
            TResourceContext& secondary_resource_context);
protected:
    static THREAD_LOCAL(TResourceContext*) primary_resource_context_;
    static THREAD_LOCAL(TResourceContext*) secondary_resource_context_;
};

}
