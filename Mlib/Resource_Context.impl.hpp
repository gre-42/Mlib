#pragma once
#include "Resource_Context.hpp"
#include <Mlib/Throw_Or_Abort.hpp>
#include <iostream>

namespace Mlib {

template <class TResourceContext>
ResourceContextGuard<TResourceContext>::ResourceContextGuard(TResourceContext& resource_context)
: resource_context_{resource_context},
  old_primary_resource_context_{ResourceContextStack<TResourceContext>::primary_resource_context_},
  old_secondary_resource_context_{ResourceContextStack<TResourceContext>::secondary_resource_context_}
{
    if (old_primary_resource_context_ == nullptr) {
        ResourceContextStack<TResourceContext>::primary_resource_context_ = &resource_context_;
    }
    ResourceContextStack<TResourceContext>::secondary_resource_context_ = &resource_context_;
}

template <class TResourceContext>
ResourceContextGuard<TResourceContext>::~ResourceContextGuard() {
    ResourceContextStack<TResourceContext>::primary_resource_context_ = old_primary_resource_context_;
    ResourceContextStack<TResourceContext>::secondary_resource_context_ = old_secondary_resource_context_;
}

template <class TResourceContext>
TResourceContext& ResourceContextStack<TResourceContext>::primary_resource_context() {
    if (primary_resource_context_ == nullptr) {
        THROW_OR_ABORT("Primary resource_context on empty stack");
    }
    return *primary_resource_context_;
}

template <class TResourceContext>
TResourceContext& ResourceContextStack<TResourceContext>::resource_context() {
    if (secondary_resource_context_ == nullptr) {
        THROW_OR_ABORT("Secondary resource_context on empty stack");
    }
    return *secondary_resource_context_;
}

template <class TResourceContext>
std::function<std::function<void()>(std::function<void()>)>
    ResourceContextStack<TResourceContext>::generate_thread_runner(
        TResourceContext& primary_resource_context,
        TResourceContext& secondary_resource_context)
{
    return [primary_resource_context, secondary_resource_context](std::function<void()> f){
        return [primary_resource_context, secondary_resource_context, f](){
            ResourceContextGuard<TResourceContext> rrg0{const_cast<TResourceContext&>(primary_resource_context)};
            ResourceContextGuard<TResourceContext> rrg1{const_cast<TResourceContext&>(secondary_resource_context)};
            f();
        };
    };
}

template <class TResourceContext>
THREAD_LOCAL(TResourceContext*) ResourceContextStack<TResourceContext>::primary_resource_context_ = nullptr;

template <class TResourceContext>
THREAD_LOCAL(TResourceContext*) ResourceContextStack<TResourceContext>::secondary_resource_context_ = nullptr;

}
