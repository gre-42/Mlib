#pragma once
#include <exception>

namespace Mlib {

void add_unhandled_exception(std::exception_ptr ptr);

bool unhandled_exceptions_occured();

void print_unhandled_exceptions();

}
