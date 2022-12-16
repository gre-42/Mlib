include(../CMakeCommands.cmake)

set(mlib_excluded_files "")

my_add_library(MlibCppHttplib "1" "" "${mlib_excluded_files}")

if (OPENSSL_FOUND)
    target_link_libraries(MlibCppHttplib OpenSSL::SSL)
    target_compile_definitions(MlibCppHttplib PUBLIC CPPHTTPLIB_OPENSSL_SUPPORT)
endif()
if (MINGW)
    # https://github.com/yhirose/cpp-httplib/issues/37
    # https://docs.microsoft.com/en-us/windows/win32/api/wincrypt/nf-wincrypt-certopenstore
    # https://social.msdn.microsoft.com/Forums/en-US/b0faf4bc-5949-409c-aaf8-c5541710a4b6/using-wincrypth?forum=Vsexpressvc
    target_link_libraries(MlibCppHttplib -lws2_32 -ladvapi32 -lcrypt32)
endif()
