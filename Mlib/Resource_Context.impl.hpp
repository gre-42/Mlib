#pragma once
#include "Resource_Context.hpp"
#include <ostream>

using namespace Mlib;

template <class TResourceContext>
ResourceContextGuard<TResourceContext>::ResourceContextGuard(const TResourceContext& resource_context) {
    ResourceContextStack<TResourceContext>::resource_context_stack().push_back(resource_context);
}

template <class TResourceContext>
ResourceContextGuard<TResourceContext>::~ResourceContextGuard() {
    if (ResourceContextStack<TResourceContext>::resource_context_stack().empty()) {
        #ifdef __GNUC__
            #pragma GCC push_options
            #pragma GCC diagnostic ignored "-Wterminate"
        #endif
        throw std::runtime_error("~ResourceContextGuard but stack is empty");
        #ifdef __GNUC__
            #pragma GCC pop_options
        #endif
    }
    ResourceContextStack<TResourceContext>::resource_context_stack().pop_back();
}

template <class TResourceContext>
TResourceContext ResourceContextStack<TResourceContext>::primary_resource_context() {
    if (ResourceContextStack::resource_context_stack().empty()) {
        throw std::runtime_error("Primary resource_context on empty stack");
    }
    return ResourceContextStack::resource_context_stack().front();
}

template <class TResourceContext>
TResourceContext ResourceContextStack<TResourceContext>::resource_context() {
    if (ResourceContextStack::resource_context_stack().empty()) {
        throw std::runtime_error("Secondary resource_context on empty stack");
    }
    return ResourceContextStack::resource_context_stack().back();
}

template <class TResourceContext>
std::function<std::function<void()>(std::function<void()>)>
    ResourceContextStack<TResourceContext>::generate_thread_runner(
        const TResourceContext& primary_resource_context,
        const TResourceContext& secondary_resource_context)
{
    return [primary_resource_context, secondary_resource_context](std::function<void()> f){
        return [primary_resource_context, secondary_resource_context, f](){
            ResourceContextGuard<TResourceContext> rrg0{primary_resource_context};
            ResourceContextGuard<TResourceContext> rrg1{secondary_resource_context};
            f();
        };
    };
}

template <class TResourceContext>
void ResourceContextStack<TResourceContext>::print_stack(std::ostream& ostr) {
    ostr << "ResourceContext stack\n";
    size_t i = 0;
    for (const auto& e : resource_context_stack()) {
        ostr << "Stack element " << i++ << '\n';
        ostr << e << '\n';
    };
}

template <class TResourceContext>
std::list<TResourceContext>& ResourceContextStack<TResourceContext>::resource_context_stack() {
    static thread_local std::list<TResourceContext> result;
    return result;
}
