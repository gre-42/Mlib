#include "Pretty_Terminate.hpp"
#include <exception>

#ifdef __ANDROID__
#include <NDKHelper.h>

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
            LOGE("terminate called after throwing an instance of \"%s\". what(): \"%s\".",
                 typeid(ex).name(),
                 ex.what());
        }
        catch (...)
        {
            LOGE("terminate called after throwing an instance of \"%s\".",
                 typeid(std::current_exception()).name());
        }
        LOGE("errno: %d: \"%s\".", errno, std::strerror(errno));
        std::abort();
    });
}
#else
#include <cstring>
#include <iostream>

using namespace Mlib;

void Mlib::register_pretty_terminate() {
    // From: https://stackoverflow.com/questions/4366739/global-exception-handling-in-c
    std::set_terminate([]() -> void {
        std::cerr << "terminate called after throwing an instance of ";
        try
        {
            std::rethrow_exception(std::current_exception());
        }
        catch (const std::exception &ex)
        {
            std::cerr << typeid(ex).name() << std::endl;
            std::cerr << "  what(): " << ex.what() << std::endl;
        }
        catch (...)
        {
            std::cerr << typeid(std::current_exception()).name() << std::endl;
            std::cerr << " ...something, not an exception, dunno what." << std::endl;
        }
        std::cerr << "errno: " << errno << ": " << std::strerror(errno) << std::endl;
        std::abort();
    });
}

#endif
