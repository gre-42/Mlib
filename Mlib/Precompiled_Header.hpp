#pragma once

#ifdef __GNUC__

// Include less because of this bug: https://gcc.gnu.org/bugzilla/show_bug.cgi?id=64117
#include <Mlib/Json/Base.hpp>

#else

#include <Mlib/Json/Json_View.hpp>
#include <Mlib/Json/Misc.hpp>
#include <Mlib/Math/Fixed_Math.hpp>
#include <Mlib/Math/Math.hpp>
#include <Mlib/Math/Simd.hpp>
#include <Mlib/Uninitialized.hpp>

#endif
