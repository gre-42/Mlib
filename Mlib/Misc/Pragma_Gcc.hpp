#pragma once

#if defined(__GNUC__) && !defined(__clang__) && !defined(__INTEL_COMPILER)
    // From: https://stackoverflow.com/questions/7608229/preprocessor-token-pasting-in-gcc-s-pragma-operator
    #define DO_GCC_PRAGMA(x)                    _Pragma(#x)
    #define PRAGMA_GCC_DIAGNOSTIC_IGNORED(x)    DO_GCC_PRAGMA(GCC diagnostic ignored #x)
    #define PRAGMA_GCC_DIAGNOSTIC_PUSH          DO_GCC_PRAGMA(GCC diagnostic push)
    #define PRAGMA_GCC_DIAGNOSTIC_POP           DO_GCC_PRAGMA(GCC diagnostic pop)

    #define PRAGMA_GCC_O3_BEGIN                 DO_GCC_PRAGMA(GCC push_options)     \
                                                DO_GCC_PRAGMA(GCC optimize("O3"))
    #define PRAGMA_GCC_O3_END                   DO_GCC_PRAGMA(GCC pop_options)
#else
    #define PRAGMA_GCC_DIAGNOSTIC_IGNORED(x)
    #define PRAGMA_GCC_DIAGNOSTIC_PUSH
    #define PRAGMA_GCC_DIAGNOSTIC_POP

    #define PRAGMA_GCC_O3_BEGIN
    #define PRAGMA_GCC_O3_END
#endif
