include(../CMakeCommands.cmake)

set(mlib_excluded_files "")

my_add_shared_library(MlibCppHttplib "1" "" "${mlib_excluded_files}")

if (OPENSSL_FOUND)
    target_link_libraries(MlibCppHttplib OpenSSL::SSL)
endif()
