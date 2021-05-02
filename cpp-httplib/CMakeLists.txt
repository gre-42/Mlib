include(../CMakeCommands.cmake)

set(mlib_excluded_files "")

my_add_shared_library(MlibCppHttplib "1" "" "${mlib_excluded_files}")

set(libs)
if (OPENSSL_FOUND)
    list(APPEND libs OpenSSL::SSL)
endif()
if (MINGW)
    # https://github.com/yhirose/cpp-httplib/issues/37
    list(APPEND libs -lws2_32)
endif()

target_link_libraries(MlibCppHttplib ${libs})
