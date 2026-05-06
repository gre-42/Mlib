#include "Termination_Manager.hpp"
#include <Mlib/Os/Os.hpp>
#include <list>
#include <mutex>
#include <shared_mutex>

using namespace Mlib;

static std::list<std::exception_ptr> unhandled_exceptions;
static std::shared_mutex mutex;

void Mlib::add_unhandled_exception(std::exception_ptr ptr) {
    std::scoped_lock lock{mutex};
    unhandled_exceptions.push_back(ptr);
}

bool Mlib::unhandled_exceptions_occured() {
    std::shared_lock lock{mutex};
    return !unhandled_exceptions.empty();
}

void Mlib::print_unhandled_exceptions() {
    std::shared_lock lock{mutex};
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
    std::shared_lock lock{mutex};
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
