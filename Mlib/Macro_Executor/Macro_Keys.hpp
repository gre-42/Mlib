#pragma once
#include <Mlib/Argument_List.hpp>
#include <set>

namespace Mlib {

namespace MacroKeys {
BEGIN_ARGUMENT_LIST;
// One of
DECLARE_ARGUMENT(call);
DECLARE_ARGUMENT(declare_macro);
DECLARE_ARGUMENT(playback);
DECLARE_ARGUMENT(execute);
DECLARE_ARGUMENT(include);
DECLARE_ARGUMENT(comment);
// Only macro
DECLARE_ARGUMENT(content);
// Macro and function
DECLARE_ARGUMENT(required);
DECLARE_ARGUMENT(exclude);
DECLARE_ARGUMENT(context);
DECLARE_ARGUMENT(arguments);
DECLARE_ARGUMENT(let);
DECLARE_ARGUMENT(without);
}

namespace UserKeys {
BEGIN_ARGUMENT_LIST;
DECLARE_ARGUMENT(title);
DECLARE_ARGUMENT(charset);
}

static std::set<std::string_view> unexpanded_keys =
{
    UserKeys::title,
    UserKeys::charset,
    MacroKeys::required,
    MacroKeys::exclude,
    MacroKeys::arguments,
    MacroKeys::content,
    MacroKeys::comment
};

}
