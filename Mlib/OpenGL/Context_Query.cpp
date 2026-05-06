#include "Context_Query.hpp"
#include <Mlib/OpenGL/IContext.hpp>
#include <stdexcept>

using namespace Mlib;

IContext* ContextQuery::context_ = nullptr;

void ContextQuery::set_context(IContext& context) {
    if (context_ != nullptr) {
        throw std::runtime_error("Global context already set");
    }
    context_ = &context;
}

void ContextQuery::clear_context() {
    if (context_ == nullptr) {
        throw std::runtime_error("clear_context() called, but global window not set");
    }
    context_ = nullptr;
}

IContext& ContextQuery::context() {
    if (context_ == nullptr) {
        throw std::runtime_error("context() called, but global window not set");
    }
    return *context_;
}

bool ContextQuery::is_initialized() {
    return context().is_initialized();
}

ContextQueryGuard::ContextQueryGuard(IContext& context) {
    ContextQuery::set_context(context);
}

ContextQueryGuard::~ContextQueryGuard() {
    ContextQuery::clear_context();
}
