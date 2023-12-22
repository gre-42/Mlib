include(../../CMakeCommands.cmake)

my_add_executable(bullet_contacts "1")

target_include_directories(bullet_contacts PRIVATE ${Mlib_INCLUDE_DIR} ${BULLET_INCLUDE_DIR})

target_link_libraries(bullet_contacts ${BULLET_LIBRARIES})

target_compile_definitions(bullet_contacts PRIVATE ${BULLET_DEFINITIONS})

set_property(TARGET bullet_contacts PROPERTY
             MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
