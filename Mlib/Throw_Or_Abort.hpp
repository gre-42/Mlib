#pragma once

#ifdef __ANDROID__
#include <Mlib/Os.hpp>
#define THROW_OR_ABORT(message) ::Mlib::verbose_abort(message)
#else
#include <stdexcept>
#define THROW_OR_ABORT(message) throw ::std::runtime_error(message)
#endif