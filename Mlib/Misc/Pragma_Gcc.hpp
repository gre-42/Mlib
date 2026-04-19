#pragma once

#ifdef _MSC_VER
#define PRAGMA_GCC_IGNORED(x)
    #define PRAGMA_GCC_DIAGNOSTIC_PUSH
    #define PRAGMA_GCC_DIAGNOSTIC_POP
#else
    // From: https://stackoverflow.com/questions/7608229/preprocessor-token-pasting-in-gcc-s-pragma-operator
    #define DO_PRAGMA(x) _Pragma(#x)
    #define PRAGMA_GCC_DIAGNOSTIC_IGNORED(x)    DO_PRAGMA(GCC diagnostic ignored #x)
    #define PRAGMA_GCC_DIAGNOSTIC_PUSH          DO_PRAGMA(GCC diagnostic push)
    #define PRAGMA_GCC_DIAGNOSTIC_POP           DO_PRAGMA(GCC diagnostic pop)
#endif
