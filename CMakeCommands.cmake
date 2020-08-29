include(CheckCXXCompilerFlag)

#####################
# Utility functions #
#####################

# parent drectory of a file
macro(my_file_parent_dir parentDir file)
    string(REGEX REPLACE "(.*)/(.*)" "\\1" ${parentDir} ${file})
endmacro()

macro(my_list_all_parent_dirs parentDirs dir)
  string(REGEX MATCH "/" ress "${dir}")

  if (ress)
    my_file_parent_dir(_parentDir_ ${dir})
    list(APPEND ${parentDirs} ${_parentDir_})
    my_list_all_parent_dirs("${parentDirs}" "${_parentDir_}")
  endif()
endmacro()

macro(my_backslash2slash varName)
    STRING(REGEX REPLACE "\\\\" "/" ${varName} ${${varName}})
    # not yet tested:
    # FILE(TO_CMAKE_PATH "${qt_headers}" qt_headers)
endmacro()

# replaces all backslashes by slashes in the environment variable with name ${varName}
macro(my_backslash2slash_env varName)
    set(envTmp $ENV{${varName}})
    my_backslash2slash(envTmp)
    set(ENV{${varName}} ${envTmp})
endmacro()

##############
# Find files #
##############

macro(my_find_include_and_src_files includeFiles sourceFiles isRecursive rootDirs)
    # seems to be non-recursive
    # aux_source_directory("." tempSources)

    if(${isRecursive})
        set(globType GLOB_RECURSE)
    else()
        set(globType GLOB)
    endif()
    set(globExpressions_h "")
    set(globExpressions_c "")
    foreach(rootDir ${rootDirs})
        list(APPEND globExpressions_h "${rootDir}/*.h" "${rootDir}/*.hpp" "${rootDir}/*.hxx")
        list(APPEND globExpressions_c "${rootDir}/*.c" "${rootDir}/*.cpp" "${rootDir}/*.cxx")
    endforeach()
    file(${globType} ${includeFiles} ${globExpressions_h})
    file(${globType} ${sourceFiles}  ${globExpressions_c})
endmacro()

macro(my_remove_excluded_files dstFileList srcFileList excludedFiles)
    set(filter_tmp "")
    foreach(l ${srcFileList})
        set(toBeRemoved FALSE)
        foreach(e ${excludedFiles})
            if("${l}" MATCHES ".*${e}.*")
                set(toBeRemoved TRUE)
            endif()
        endforeach()
        if(NOT toBeRemoved)
            set(filter_tmp ${filter_tmp} ${l})
        else()
            # message("removing " ${l})
        endif()
    endforeach()
    set(${dstFileList} "${filter_tmp}")
endmacro()

macro(my_list_directories_any_dir dirList currentDir)
    # file(GLOB_RECURSE tempFilesAndDirs "*")
    # file(GLOB sub-dir RELATIVE ${curdir} *)

    # GLOB_RECURSE did not add directories. use GLOB and a recursive call instead
    set(tempFilesAndDirs "")
    file(GLOB tempFilesAndDirs ${currentDir}/*)

    foreach(dir ${tempFilesAndDirs})
        if(IS_DIRECTORY ${dir})
            list(APPEND dirList ${dir})
            # recursive call
            my_list_directories_any_dir(dirList ${dir})
        endif()
    endforeach()
endmacro()

#####################
# Add source groups #
#####################

macro(my_relPath2SourceGroup sourceGroup relPath)
    string(REPLACE "/" "\\\\" "${sourceGroup}" "${relPath}")
endmacro()

# Searches the current src dir and adds source groups for all files that can be found
macro(my_add_source_groups_v1)
    my_list_directories_any_dir(dirList ${CMAKE_CURRENT_SOURCE_DIR})
    foreach(dir ${dirList})

        file(RELATIVE_PATH relativeDir ${CMAKE_CURRENT_SOURCE_DIR} ${dir})
        my_relPath2SourceGroup(filterStr, ${relativeDir})
        # string(REGEX REPLACE "(.*)/(.*)" "\\1${backSlash}${backSlash}\\2" endd ${relativeDir})

        set(includeFiles)
        file(GLOB includeFiles ${dir}/*.h ${dir}/*.hpp ${dir}/*.hxx)
        set(sourceFiles)
        file(GLOB sourceFiles ${dir}/*.c ${dir}/*.cpp ${dir}/*.cxx)
        source_group("Header Files\\${filterStr}" FILES ${includeFiles})
        source_group("Source Files\\${filterStr}" FILES ${sourceFiles})
    endforeach()
endmacro()

macro(my_add_source_groups_v2 rootFilter rootDir fileList)

  # foreach(file ${fileList})
    # my_file_parent_dir(parentDir ${file})

    # file(RELATIVE_PATH relParentDir ${rootDir} ${parentDir})

    # #message("file ${file}")
    # # visual studio filters do not work if a parent filter does not contain any file, so add a dummy file for each parent directory
    # # see http://www.vtk.org/Bug/view.php?id=10555
    # set(relParentDir_parents "")
    # my_list_all_parent_dirs(relParentDir_parents "${relParentDir}")
    # foreach(relParentDir_parent ${relParentDir_parents})
      # #message("par ${relParentDir_parent}")
      # #message("pd ${parentDir}")
      # my_relPath2SourceGroup(relParentDir_parent_sourceGroup "${relParentDir_parent}")
      # #source_group("${rootFilter}\\${relParentDir_parent_sourceGroup}" FILES "${parentDir}/dummy.x")
      # #message("adding ${rootFilter}\\${relParentDir_parent_sourceGroup}")
      # #message("ff ${file}")
      # list(APPEND ${fileList} "${parentDir}/dummy.x")
      # #message("append ${parentDir}/dummy.x")
    # endforeach()
  # endforeach()

    foreach(file ${fileList})

        my_file_parent_dir(parentDir ${file})

        file(RELATIVE_PATH relParentDir ${rootDir} ${parentDir})
        my_relPath2SourceGroup(sourceGroup "${relParentDir}")
        source_group("${rootFilter}\\${sourceGroup}" FILES ${file})

    endforeach()
endmacro()

# recursively (if isRecursive is true) searches the directory rootDir for library files and creates a list that can be passed to target_link_libraries
macro(my_find_link_libraries libListName rootDir debugPostfix isRecursive filenames)
    # find_library(${libListRelease} "*_d*" PATHS ${rootDir})

    # create list of filters (rootDir/filename[_d].lib)
    set(absReleaseFilters "")
    set(absDebugFilters "")
    foreach(filename ${filenames})
        #message("fn ${filename}")
        set(absReleaseFilters ${absReleaseFilters}  "${rootDir}/${filename}${CMAKE_LINK_LIBRARY_SUFFIX}")
        set(absDebugFilters ${absDebugFilters}  "${rootDir}/${filename}${debugPostfix}${CMAKE_LINK_LIBRARY_SUFFIX}")
    endforeach()

    # message("relfilt ${absReleaseFilters}")
    # message("debfilt ${absDebugFilters}")
    set(libListRelease "")
    set(libListDebug "")

    if(${isRecursive})
        set(globType GLOB_RECURSE)
    else()
        set(globType GLOB)
    endif()
    # find files using the filters
    file(${globType} libListRelease ${absReleaseFilters})
    file(${globType} libListDebug ${absDebugFilters})

    # remove all files from the release libraries that are also in the debug version
    # provided a debug postfix was set (otherwise the debug and release libs are the same)
    list( LENGTH libListDebug libListDebugLen )
    if(${libListDebugLen} GREATER 0)
        if(NOT ${debugPostfix} EQUAL "")
            list(REMOVE_ITEM libListRelease ${libListDebug})
        endif()
    endif()

    set(${libListName} "")
    # build the final list
    foreach(file ${libListRelease})
        list(APPEND ${libListName} optimized ${file})
    endforeach()
    foreach(file ${libListDebug})
        list(APPEND ${libListName} debug ${file})
    endforeach()

    # message("isrec ${isRecursive} globt ${globType}")
    # message("llist ${${libListName}}")
    # message("rell ${libListRelease}")
    # message("debb ${libListDebug}")
endmacro()

######################
# Add Lib/Executable #
######################

# type can be STATIC, SHARED or MODULE
macro(my_add_advanced_library libName type isRecursive additionalFiles excludedFiles)
    set(incFiles)
    set(srcFiles)

    my_find_include_and_src_files(incFiles srcFiles ${isRecursive} ${CMAKE_CURRENT_SOURCE_DIR})

    my_remove_excluded_files(incFiles "${incFiles}" "${excludedFiles}")
    my_remove_excluded_files(srcFiles "${srcFiles}" "${excludedFiles}")

    my_add_source_groups_v2("Header Files" ${CMAKE_CURRENT_SOURCE_DIR} "${incFiles}")
    my_add_source_groups_v2("Source Files" ${CMAKE_CURRENT_SOURCE_DIR} "${srcFiles}")

    add_library( ${libName} ${type} ${incFiles} ${srcFiles} ${additionalFiles})
endmacro()

macro(my_add_library libName isRecursive additionalFiles excludedFiles)
    my_add_advanced_library(${libName} STATIC ${isRecursive} "${additionalFiles}" "${excludedFiles}")
endmacro()

macro(my_add_shared_library libName isRecursive additionalFiles excludedFiles)
    my_add_advanced_library(${libName} SHARED ${isRecursive} "${additionalFiles}" "${excludedFiles}")
endmacro()

# build static and shared libraries in the same build
macro(my_add_object_library libName isRecursive additionalFiles excludedFiles)
    if (WIN32) #Windows
        my_add_advanced_library(${libName} SHARED ${isRecursive} "${additionalFiles}" "${excludedFiles}")
    else () #Unix
        my_add_advanced_library(${libName}_objlib OBJECT ${isRecursive} "${additionalFiles}" "${excludedFiles}")
        # http://stackoverflow.com/questions/2152077/is-it-possible-to-get-cmake-to-build-both-a-static-and-shared-version-of-the-sam
        # https://datainfer.wordpress.com/2013/10/24/make-both-static-and-shared-libraries-in-one-build-with-cmake/
        add_library(${libName}_static STATIC $<TARGET_OBJECTS:${libName}_objlib>)
        set_target_properties(${libName}_static PROPERTIES OUTPUT_NAME ${libName})
        add_library(${libName} SHARED $<TARGET_OBJECTS:${libName}_objlib>)
    endif()
endmacro()

macro(my_add_executable libName isRecursive)
    set(incFiles)
    set(srcFiles)

    my_find_include_and_src_files(incFiles srcFiles ${isRecursive} ${CMAKE_CURRENT_SOURCE_DIR})

    my_add_source_groups_v2("Header Files" ${CMAKE_CURRENT_SOURCE_DIR} "${incFiles}")
    my_add_source_groups_v2("Source Files" ${CMAKE_CURRENT_SOURCE_DIR} "${srcFiles}")

    if ("${srcFiles}" STREQUAL "")
        message(FATAL_ERROR "Could not find a single source file for ${libName} in directory ${CMAKE_CURRENT_SOURCE_DIR}")
    endif()
    message("Adding executable ${libName}, Includes: ${incFiles} Sources: ${srcFiles}")

    add_executable( ${libName} ${incFiles} ${srcFiles})
endmacro()

macro(my_set_pyd targetName)
    if (WIN32) #Windows
        set_target_properties(${targetName} PROPERTIES DEBUG_POSTFIX "" PREFIX "" SUFFIX .pyd)
    else () #Unif
        set_target_properties(${targetName} PROPERTIES DEBUG_POSTFIX "" PREFIX "" SUFFIX .so)
    endif()
endmacro()

# http://public.kitware.com/pipermail/cmake/2015-October/061889.html
macro(AddCXXFlagIfSupported flag test)
   CHECK_CXX_COMPILER_FLAG(${flag} ${test})
   if( ${${test}} )
      message("Adding flag ${flag}")
      set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${flag}")
   endif()
endmacro()

macro(warn_all)
    # http://stackoverflow.com/a/14235055/2292832
    # http://stackoverflow.com/a/21561742/2292832
    # if ( CMAKE_COMPILER_IS_GNUCC ) # not working
    if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
        # This is tested
        add_compile_options( -Wall -Wno-unknown-pragmas -Werror )
    endif ()
    if ( MSVC )
        # This is untested
        add_compile_options( /W3 )
    endif ()
endmacro()

macro(ddebug)
    if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
        set(CMAKE_CXX_FLAGS_DEBUG "-DDEBUG -g -O0")
        # MSYS has a large function-call overhead, I guess. => O3
        if("${CMAKE_SYSTEM_NAME}" STREQUAL "MSYS")
            set(CMAKE_CXX_FLAGS_RELEASE "-DNDEBUG -O3")
        else()
            set(CMAKE_CXX_FLAGS_RELEASE "-DNDEBUG -O2")
        endif()
    endif()
endmacro()

macro(optional_omp)
    find_package(OpenMP)
    if (OPENMP_FOUND)
        set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
        set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
    endif()
endmacro()

# http://public.kitware.com/pipermail/cmake/2015-October/061889.html
macro(add_compiler_color)
    if (${CMAKE_GENERATOR} STREQUAL "Ninja")
        AddCXXFlagIfSupported(-fdiagnostics-color=always COMPILER_SUPPORTS_fdiagnostics-color)
    endif()
    if(MSVC)
        set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MP")
    endif()
endmacro()
