#pragma once
#include <cstddef>

namespace Mlib {

// From: https://stackoverflow.com/questions/13180842/how-to-calculate-offset-of-a-class-member-at-compile-time
template<typename T, typename U>
constexpr void* field_offset(U T::*member)
{
    return (void*)&((T*)nullptr->*member);
}

}
