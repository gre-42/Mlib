#pragma once
#include <exception>
#include <iosfwd>

namespace Mlib {

void add_unhandled_exception(std::exception_ptr ptr);

bool unhandled_exceptions_occured();

void print_unhandled_exceptions();
void print_unhandled_exceptions(std::ostream& ostr);

}
