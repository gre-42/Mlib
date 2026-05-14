include(../../CMakeCommands.cmake)

if (BUILD_GRAPHICS)
    my_add_library(
        NAME MlibOpenGL
        RECURSIVE
        ADDITIONAL_FILES ""
        EXCLUDED_FILES ""
    )
    # https://issuetracker.google.com/issues/214811726
    if (NOT ANDROID)
        target_precompile_headers(MlibOpenGL PRIVATE ../Misc/Precompiled_Header.hpp)
    endif()

    target_include_directories(MlibOpenGL PUBLIC ${glfw3_INCLUDE_DIR})

    target_link_libraries(MlibOpenGL
        PUBLIC
            MlibMap
            MlibGeometry
            MlibImages
            MlibLayout
            MlibMemory
            MlibMisc
            MlibMacroExecutor
            MlibNvDds
            MlibProctree
            MlibStats
            MlibSceneGraph
            MlibStb
            MlibTime)

    if (NOT EMSCRIPTEN)
        target_link_libraries(MlibOpenGL
            PUBLIC
                MlibGlad)
    else()
        target_link_against_opengl(MlibOpenGL)
    endif()
else()
    my_add_library(
        NAME MlibOpenGL
        RECURSIVE
        ADDITIONAL_FILES
            Batch_Renderers/Particle_Creator.cpp
            Batch_Renderers/Particle_Creator.hpp
            Batch_Renderers/Particle_Renderer.cpp
            Batch_Renderers/Particle_Renderer.hpp
            Batch_Renderers/Particles_Instance.cpp
            Batch_Renderers/Particles_Instance.hpp
            Batch_Renderers/Trail_Renderer.cpp
            Batch_Renderers/Trail_Renderer.hpp
            Batch_Renderers/Trails_Instance.cpp
            Batch_Renderers/Trails_Instance.hpp
            Modifiers/Add_Foliage_Resource.cpp
            Modifiers/Add_Foliage_Resource.hpp
            Modifiers/Cluster_Elements.cpp
            Modifiers/Cluster_Elements.hpp
            Modifiers/Replace_Terrain_Material.cpp
            Modifiers/Replace_Terrain_Material.hpp
            Renderables/Renderable_Colored_Vertex_Array.cpp
            Renderables/Renderable_Colored_Vertex_Array.hpp
            Resources/Bvh_File_Resource.cpp
            Resources/Bvh_File_Resource.hpp
            Resources/Colored_Vertex_Array_Resource.cpp
            Resources/Colored_Vertex_Array_Resource.hpp
            Resources/Colored_Vertex_Array_Resource/Trail_Storage.cpp
            Resources/Colored_Vertex_Array_Resource/Trail_Storage.hpp
            Resource_Managers/Particle_Resources.cpp
            Resource_Managers/Particle_Resources.hpp
            Resource_Managers/Trail_Resources.cpp
            Resource_Managers/Trail_Resources.hpp
            Resources/Colored_Vertex_Array_Resource/Trail_Extender.cpp
            Resources/Colored_Vertex_Array_Resource/Trail_Extender.hpp
            Resources/Foliage_Resource.cpp
            Resources/Foliage_Resource.hpp
            Resources/Dff_File_Resource.cpp
            Resources/Dff_File_Resource.hpp
            Resources/Heterogeneous_Resource.cpp
            Resources/Heterogeneous_Resource.hpp
            Resources/Kn5_File_Resource.cpp
            Resources/Kn5_File_Resource.hpp
            Resources/Mhx2_File_Resource.cpp
            Resources/Mhx2_File_Resource.hpp
            Resources/Obj_File_Resource.cpp
            Resources/Obj_File_Resource.hpp
            Resources/Proctree_File_Resource.cpp
            Resources/Proctree_File_Resource.hpp
            Resources/Pssg_File_Resource.cpp
            Resources/Pssg_File_Resource.hpp
            Selected_Cameras/Camera_Cycle.cpp
            Selected_Cameras/Camera_Cycle.hpp
            Selected_Cameras/Selected_Cameras.cpp
            Selected_Cameras/Selected_Cameras.hpp
        EXCLUDED_FILES ".*"
    )
    # https://issuetracker.google.com/issues/214811726
    if (NOT ANDROID)
        target_precompile_headers(MlibOpenGL PRIVATE ../Misc/Precompiled_Header.hpp)
    endif()

    target_link_libraries(MlibOpenGL
        PUBLIC
            MlibCereal
            MlibJson
            MlibProctree
            MlibResourceContext
            MlibSceneGraph
            )
endif()
