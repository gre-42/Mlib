#pragma once
#include <Mlib/Features.hpp>

#ifdef WITHOUT_EXCEPTIONS
#include <Mlib/Os/Os.hpp>
#define THROW_OR_ABORT(message) ::Mlib::verbose_abort(message)
#define THROW_OR_ABORT2(exception) ::Mlib::verbose_abort(exception.what())
#else
#include <stdexcept>
#define THROW_OR_ABORT(message) throw ::std::runtime_error(message)
#define THROW_OR_ABORT2(exception) throw exception
#endif
