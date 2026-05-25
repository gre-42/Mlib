#pragma once
#include <condition_variable>
#include <exception>
#include <iosfwd>

namespace Mlib {

template <class T>
class ThreadSafePromise;

void add_unhandled_exception(std::exception_ptr ptr);

bool unhandled_exceptions_occured();

void print_unhandled_exceptions();
void print_unhandled_exceptions(std::ostream& ostr);
void convert_sigterm_to_exception();

class TerminationNotificationGuardCv {
public:
    explicit TerminationNotificationGuardCv(std::condition_variable& cv);
    ~TerminationNotificationGuardCv();
private:
    std::condition_variable& cv_;
};

class TerminationNotificationGuardPromise {
public:
    explicit TerminationNotificationGuardPromise(
        ThreadSafePromise<void>& promise);
    ~TerminationNotificationGuardPromise();
private:
    ThreadSafePromise<void>& promise_;
};

}
