#pragma once

#ifdef _MSC_VER
    // From: https://stackoverflow.com/questions/7608229/preprocessor-token-pasting-in-gcc-s-pragma-operator
    #define DO_MSC_PRAGMA(x)                    _Pragma(#x)
    #define PRAGMA_MSC_WARNING_DISABLE(x)       DO_MSC_PRAGMA(warning(disable: x))
    #define PRAGMA_MSC_WARNING_PUSH             DO_MSC_PRAGMA(warning(push))
    #define PRAGMA_MSC_WARNING_POP              DO_MSC_PRAGMA(warning(pop))
#else
    #define PRAGMA_MSC_WARNING_DISABLE(x)
    #define PRAGMA_MSC_WARNING_PUSH
    #define PRAGMA_MSC_WARNING_POP
#endif
