#include "Termination_Manager.hpp"
#include <Mlib/Os/Os.hpp>
#include <csignal>
#include <list>
#include <mutex>
#include <shared_mutex>
#include <unordered_set>

using namespace Mlib;

static std::list<std::exception_ptr> unhandled_exceptions;
static std::shared_mutex unhandled_exceptions_mutex;

static std::unordered_set<std::condition_variable*> exception_observers;
static std::mutex exception_observers_mutex;

void Mlib::add_unhandled_exception(std::exception_ptr ptr) {
    {
        std::scoped_lock lock{unhandled_exceptions_mutex};
        unhandled_exceptions.push_back(ptr);
    }
    {
        std::scoped_lock lock{exception_observers_mutex};
        for (auto& cv : exception_observers) {
            cv->notify_all();
        }
    }
}

bool Mlib::unhandled_exceptions_occured() {
    std::shared_lock lock{unhandled_exceptions_mutex};
    return !unhandled_exceptions.empty();
}

void Mlib::print_unhandled_exceptions() {
    std::shared_lock lock{unhandled_exceptions_mutex};
    if (!unhandled_exceptions.empty()) {
        lerr() << unhandled_exceptions.size() << " unhandled exception(s)";
        for (const auto& e : unhandled_exceptions) {
            try {
                std::rethrow_exception(e);
            } catch (const std::exception& ex) {
                lerr() << "Unhandled exception: " << ex.what();
            } catch (...) {
                lerr() << "Unhandled exception of unknown type";
            }
        }
    }
}

void Mlib::print_unhandled_exceptions(std::ostream& ostr) {
    std::shared_lock lock{unhandled_exceptions_mutex};
    if (!unhandled_exceptions.empty()) {
        ostr << unhandled_exceptions.size() << " unhandled exception(s)" << std::endl;
        for (const auto& e : unhandled_exceptions) {
            try {
                std::rethrow_exception(e);
            } catch (const std::exception& ex) {
                ostr << "Unhandled exception: " << ex.what() << std::endl;
            } catch (...) {
                ostr << "Unhandled exception of unknown type" << std::endl;
            }
        }
    }
}

static void signal_handler(int signum) {
    if (signum == SIGTERM) {
        add_unhandled_exception(std::make_exception_ptr(
            std::runtime_error("SIGTERM received, treating as unhandled exception")));
    }
}
 
void Mlib::convert_sigterm_to_exception() {
    std::signal(SIGTERM, signal_handler);
}

TerminationNotificationGuard::TerminationNotificationGuard(std::condition_variable& cv)
    : cv_{cv}
{
    std::scoped_lock lock{exception_observers_mutex};
    if (!exception_observers.insert(&cv).second) {
        throw std::runtime_error("Condition variable already registered");
    }
}

TerminationNotificationGuard::~TerminationNotificationGuard() {
    std::scoped_lock lock{exception_observers_mutex};
    if (exception_observers.erase(&cv_) != 1) {
        verbose_abort("Could not find condition variable to be removed");
    }
}
