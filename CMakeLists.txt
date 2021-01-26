cmake_minimum_required(VERSION 3.2)

project(Abclib CXX)

enable_testing()
include(CMakeCommands.cmake)
warn_all()
add_compiler_color()
ddebug()
optional_omp()
set (CMAKE_CXX_STANDARD 20)
set (CMAKE_POSITION_INDEPENDENT_CODE TRUE)

####################
# Add dependencies #
####################

# Replaced with MSYS2 package
# if (MINGW)
#     cmake_policy(SET CMP0074 NEW)
#     set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH};${CMAKE_CURRENT_SOURCE_DIR})
#     set(glfw3_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/glfw)
# endif()

if (CMAKE_GENERATOR MATCHES "Visual Studio")
    set (glfw3_LIBRARY ${CMAKE_CURRENT_SOURCE_DIR}/glfw_vc2019/lib/glfw3dll.lib)
    set (glfw3_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/glfw_vc2019/include)
    set (glfw3_FOUND true)
else()
    find_package(glfw3)
endif()
find_package(SDL2)
# MlibRender contains glad_gl.cpp and glad_vulkan.cpp
find_package(Vulkan)
find_package(glm)
find_package(OpenCV OPTIONAL_COMPONENTS features2d)
find_package(OpenSSL)

if (OpenCV_FOUND)
    add_compile_definitions(WITH_OPENCV)
endif()

if (OPENSSL_FOUND)
    add_compile_definitions(WITH_OPENSSL)
endif()

#####################
# Set ouptut pathes #
#####################

# https://stackoverflow.com/questions/33062728/cmake-link-shared-library-on-windows
if(MSVC)
    set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS TRUE)
endif()
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/Lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/Lib)
if (MINGW OR MSVC)
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/Bin)
endif()
set(CMAKE_DEBUG_POSTFIX _d)
set(Mlib_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR})

if(MSVC)
    add_compile_definitions(_USE_MATH_DEFINES)
endif()

######################
# Add subdirectories #
######################

add_subdirectory(Apps)
add_subdirectory(cpp-httplib)
add_subdirectory(Mlib)
add_subdirectory(poly2tri)
add_subdirectory(stb_image)
add_subdirectory(Tests)
# Requires 32 bit or disabled warnings.
# add_subdirectory(Triangle)
