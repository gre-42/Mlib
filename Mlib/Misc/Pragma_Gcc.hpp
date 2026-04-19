#pragma once

#ifdef _MSC_VER
    #define PRAGMA_GCC(x)
#else
    // From: https://stackoverflow.com/questions/7608229/preprocessor-token-pasting-in-gcc-s-pragma-operator
    #define DO_PRAGMA(x) _Pragma(#x)
    #define PRAGMA_GCC(x) DO_PRAGMA("GCC " #x)
#endif
