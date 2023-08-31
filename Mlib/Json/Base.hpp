#pragma once
#include <Mlib/Features.hpp>
#include <Mlib/Throw_Or_Abort.hpp>
#include <map>
#include <set>
#include <string>
#include <vector>

#ifdef WITHOUT_EXCEPTIONS
#include <Mlib/Os/Os.hpp>
#define JSON_TRY_USER if(true)
#define JSON_CATCH_USER(exception) if(false)
#define JSON_THROW_USER(exception)                           \
    {::Mlib::verbose_abort((exception).what());}
#endif

#include <nlohmann/json.hpp>
