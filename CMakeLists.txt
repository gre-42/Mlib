cmake_minimum_required(VERSION 3.18.1)

project(Mlib C CXX)

enable_testing()
include(CMakeCommands.cmake)
print_environment()
warn_all()
add_compiler_color()
ddebug()
set_stack_size()
if (NOT ANDROID)
    optional_omp()
endif()
if (ANDROID)
    enable_cross_compiling()
endif()
set_ehsc()
avoid_floating_point_exceptions()
enable_absolute_diagnostic_pathes()
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_POSITION_INDEPENDENT_CODE TRUE)

###############
# Add options #
###############

option(BUILD_TRIANGLE "Build the Triangle library" OFF)
option(BUILD_CORK "Build the Cork Boolean/CSG library" OFF)
option(BUILD_SFM "Build the structure from motion library" ON)
option(BUILD_CV "Build the computer vision library" ON)
option(BUILD_OPENCV "Build the Open Computer Vision Library" ON)
option(BUILD_SCENE "Build the scene library" ON)
option(WITH_OPENSSL "Build against OpenSSL" ON)
option(WITH_ZLIB "Build aganst zlib" ON)

####################
# Add dependencies #
####################

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/Modules/")

# Replaced with MSYS2 package
# if (MINGW)
#     cmake_policy(SET CMP0074 NEW)
#     set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH};${CMAKE_CURRENT_SOURCE_DIR})
#     set(glfw3_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/glfw)
# endif()

if (MSVC)
    set(glfw3_LIBRARY ${CMAKE_CURRENT_SOURCE_DIR}/glfw_vc2022/lib/glfw3dll.lib)
    set(glfw3_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/glfw_vc2022/include)
    set(glfw3_FOUND true)
else()
    find_package(glfw3)
endif()

set(MlibCereal_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/cereal/include)
set(MlibCereal_FOUND true)

find_package(SDL2)
# MlibRender contains glad_gl.cpp and glad_vulkan.cpp
find_package(Vulkan)
find_package(OpenCV OPTIONAL_COMPONENTS features2d)
if (WITH_OPENSSL)
	find_package(OpenSSL REQUIRED)
endif()
set(MlibEve_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/eve/include)
set(MlibEve_FOUND true)
if (MSVC)
    # Alternative OpenAL-implementation: openal.org
    # find_package(OpenAL REQUIRED)
    set(OPENAL_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/openal-soft/include/AL)
    set(OPENAL_LIBRARY ${CMAKE_CURRENT_SOURCE_DIR}/openal-soft/build/Release/OpenAL32.lib)
    set(OPENAL_FOUND true)
    set(ALUT_LIBRARY ${CMAKE_CURRENT_SOURCE_DIR}/freealut/build/src/Release/alut.lib)
    set(ALUT_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/freealut/include)
    set(ALUT_INCLUDE_DIRS ${OPENAL_INCLUDE_DIR} ${ALUT_INCLUDE_DIR})
    set(ALUT_LIBRARIES ${OPENAL_LIBRARY} ${ALUT_LIBRARY})
    set(ALUT_FOUND true)
    if (WITH_ZLIB)
        set(ZLIB_LIBRARY ${CMAKE_CURRENT_SOURCE_DIR}/VSZlibBuild/Release/zlib.lib)
        set(ZLIB_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/zlib)
        find_package(ZLIB REQUIRED)
    endif()
elseif(ANDROID)
    set(OPENAL_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/Android/openal-soft/include)
    set(OPENAL_LIBRARY ${CMAKE_CURRENT_SOURCE_DIR}/Android/openal-soft/build-${ANDROID_ABI}/libopenal.a)
    set(OPENAL_FOUND true)
    set(ALUT_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/Android/freealut/include)
    set(ALUT_INCLUDE_DIRS ${OPENAL_INCLUDE_DIR} ${ALUT_INCLUDE_DIR})
    set(ALUT_LIBRARIES -lOpenSLES ${OPENAL_LIBRARY} ${CMAKE_CURRENT_SOURCE_DIR}/Android/freealut/build-${ANDROID_ABI}/src/libalut.a)
    set(ALUT_FOUND true)
    if (WITH_ZLIB)
        set(ZLIB_LIBRARY ${CMAKE_CURRENT_SOURCE_DIR}/Android/zlib/build-${ANDROID_ABI}/libz.a)
        set(ZLIB_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/Android/zlib)
        find_package(ZLIB REQUIRED)
    endif()
else()
    find_package(OpenAL)
    find_package(ALUT)
    if (WITH_ZLIB)
        find_package(ZLIB REQUIRED)
    endif()
endif()
set(RECAST_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/recastnavigation/Recast/Include)
if (MSVC)
    set(RecastNavitation_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/VSRecastBuild)
elseif(ANDROID)
    # set(RecastNavitation_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/Android/recast/build/intermediates/cxx/${CMAKE_BUILD_TYPE}/26d71691/obj/${ANDROID_ABI})
    set(RecastNavitation_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/Android/recast/build/intermediates/stripped_native_libs/debugAndroidTest/stripDebugAndroidTestDebugSymbols/out/lib/${ANDROID_ABI})
else()
    set(RecastNavitation_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/RecastBuild)
endif()
find_package(RecastNavigation REQUIRED)

###########################
# Add compile definitions #
###########################

if (NOT (BUILD_OPENCV AND OpenCV_FOUND))
    add_compile_definitions(WITHOUT_OPENCV)
endif()

if (NOT OPENSSL_FOUND)
    add_compile_definitions(WITHOUT_OPENSSL)
endif()

if (NOT glfw3_FOUND)
    add_compile_definitions(WITHOUT_GLFW3)
endif()

if (NOT BUILD_TRIANGLE)
    add_compile_definitions(WITHOUT_TRIANGLE)
endif()

if (NOT BUILD_CORK)
    add_compile_definitions(WITHOUT_CORK)
endif()

if (NOT OPENAL_FOUND)
    add_compile_definitions(WITHOUT_OPENAL)
endif()

if (NOT ALUT_FOUND)
    add_compile_definitions(WITHOUT_ALUT)
endif()

if (NOT ZLIB_FOUND)
    add_compile_definitions(WITHOUT_ZLIB)
endif()

#####################
# Set ouptut pathes #
#####################

if (NOT ANDROID)
    if (MINGW)
        set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/Bin)
        set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/Bin)
        set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/Bin)
    elseif (MSVC)
        # https://stackoverflow.com/questions/33062728/cmake-link-shared-library-on-windows
        set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS TRUE)
        set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/Lib)
        set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/Lib)
        set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/Bin)
    else()
        set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/Lib)
        set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/Bin)
    endif()
    set(CMAKE_DEBUG_POSTFIX _d)
endif()
set(Mlib_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR})

if (MSVC)
    add_compile_definitions(_USE_MATH_DEFINES)
endif()

######################
# Add subdirectories #
######################

add_subdirectory(Apps)
if (BUILD_CORK AND (NOT (MINGW OR MSVC)))
    add_subdirectory(cork)
endif()
add_subdirectory(cpp-httplib)
add_subdirectory(glad)
add_subdirectory(half)
add_subdirectory(Mlib)
add_subdirectory(nv_dds)
add_subdirectory(poly2tri)
add_subdirectory(stb)
add_subdirectory(stb_cpp)
if (NOT ANDROID)
    add_subdirectory(Tests)
endif()
if (BUILD_TRIANGLE)
    add_subdirectory(Triangle)
endif()
