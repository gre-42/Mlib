#include "Pretty_Terminate.hpp"
#include <Mlib/Os/Os.hpp>
#include <cstring>
#include <exception>

using namespace Mlib;

void Mlib::register_pretty_terminate() {
    // From: https://stackoverflow.com/questions/4366739/global-exception-handling-in-c
    std::set_terminate([]() -> void {
        try
        {
            std::rethrow_exception(std::current_exception());
        }
        catch (const std::exception &ex)
        {
            lerr() << "terminate called after throwing an instance of \"" << typeid(ex).name() << '"';
            lerr() << "  what(): " << ex.what();
        }
        catch (...)
        {
            lerr() << "terminate called after throwing an instance of \"" << typeid(std::current_exception()).name() << '"';
        }
        lerr() << "errno: " << errno << ": " << std::strerror(errno);
        std::abort();
    });
}
