#pragma once

#ifdef __clang__
    #define DO_CLANG_PRAGMA(x)                      _Pragma(#x)
    #define PRAGMA_CLANG_DIAGNOSTIC_IGNORED(x)      DO_CLANG_PRAGMA(clang diagnostic ignored #x)
    #define PRAGMA_CLANG_DIAGNOSTIC_PUSH            DO_CLANG_PRAGMA(clang diagnostic push)
    #define PRAGMA_CLANG_DIAGNOSTIC_POP             DO_CLANG_PRAGMA(clang diagnostic pop)

    #define PRAGMA_CLANG_O3_BEGIN                   DO_CLANG_PRAGMA(clang push_options)     \
                                                    DO_CLANG_PRAGMA(clang optimize("O3"))
    #define PRAGMA_CLANG_O3_END                     DO_CLANG_PRAGMA(clang pop_options)
#else
    #define PRAGMA_CLANG_DIAGNOSTIC_IGNORED(x)
    #define PRAGMA_CLANG_DIAGNOSTIC_PUSH
    #define PRAGMA_CLANG_DIAGNOSTIC_POP

    #define PRAGMA_CLANG_O3_BEGIN
    #define PRAGMA_CLANG_O3_END
#endif
