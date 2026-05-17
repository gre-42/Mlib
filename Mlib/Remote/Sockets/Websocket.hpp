#pragma once
#include <Mlib/Misc/Pragma_Clang.hpp>
#ifdef __EMSCRIPTEN__
PRAGMA_CLANG_DIAGNOSTIC_PUSH
PRAGMA_CLANG_DIAGNOSTIC_IGNORED(-Wdeprecated-declarations)
#include <boost/beast/websocket.hpp>
PRAGMA_CLANG_DIAGNOSTIC_POP
#else
#include <boost/beast/websocket.hpp>
#endif
