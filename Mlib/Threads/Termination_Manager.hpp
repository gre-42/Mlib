#pragma once
#include <condition_variable>
#include <exception>
#include <iosfwd>

namespace Mlib {

void add_unhandled_exception(std::exception_ptr ptr);

bool unhandled_exceptions_occured();

void print_unhandled_exceptions();
void print_unhandled_exceptions(std::ostream& ostr);
void convert_sigterm_to_exception();

class TerminationNotificationGuard {
public:
    explicit TerminationNotificationGuard(std::condition_variable& cv);
    ~TerminationNotificationGuard();
private:
    std::condition_variable& cv_;
};

}
