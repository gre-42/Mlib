include(../../CMakeCommands.cmake)

set(excluded_files "")
if (NOT BUILD_AUDIO)
    list(APPEND excluded_files "/Audio/[^/]+$")
endif()
if (NOT BUILD_GRAPHICS)
    list(APPEND excluded_files
        "/Audio/[^/]+$"
        "/Key_Bindings/[^/]+$"
        "/Layout/[^/]+$"
        "/Lights/[^/]+$"
        "/Render/[^/]+$"
        "/Ui/[^/]+$"
        "/Create_Tap_Button.cpp$"
        "/Blend_Map_Texture_Json.cpp$"
        "/Acoustic_Pressure_Subdomain_Logic.cpp$"
        "/Acoustic_Velocity_Subdomain_Logic.cpp$"
        "/Create_Fluid_Subdomain.cpp$"
        "/Create_Hud_Opponent_Tracker_Logic.cpp$"
        "/Create_Acoustic_Pressure_Subdomain.cpp$"
        "/Create_Acoustic_Velocity_Subdomain.cpp$"
        "/Load_Renderable_Scene_Instance_Function.cpp$"
        "/Scene_To_Pixel_Region.cpp$"
        "/Fill_Pixel_Region_With_Texture.cpp$"
        "/Scene_To_Texture.cpp$"
        "/Create_Hud_Target_Point_Logic.cpp$"
        "/Minimap.cpp$"
        "/Create_Visual_Player_Status.cpp$"
        "/Create_Visual_Node_Status.cpp$"
        "/Create_Renderable_Scene.cpp$"
        "/Set_Camera.cpp$"
        "/Set_Camera_Cycle.cpp$"
        "/Print_Camera_Cycles.cpp$"
        "/With_Selected_Camera.cpp$"
        "/Invalidate_Aggregate_Renderers.cpp$"
        "/Focused_Text.cpp$"
        "/Create_Visual_Global_Log.cpp$"
        "/Input_State.cpp$"
        "/Thread_Top.cpp$"
        "/Create_Abs_Idle_Key_Binding.cpp$"
        "/Create_Avatar_Controller_Idle_Binding.cpp$"
        "/Instantiate_Grass.cpp$"
        "/Create_Hud_Opponent_Zoom_Logic.cpp$"
        "/Players_Stats.cpp$"
        "/Create_Skidmark.cpp$"
        "/Visual_Node_Status_3rd.cpp$"
        "/Create_Visual_Player_Bullet_Count.cpp$"
        "/Insert_Renderable_Node_Hider.cpp$"
        "/Create_Keep_Offset_From_Camera.cpp$"
        "/Add_Cubemap.cpp$"
        "/Add_Charset.cpp$"
        "/Add_Blend_Map_Texture.cpp$"
        "/Add_Texture_Descriptor.cpp$"
        "/Add_Texture_Atlas_Cpu.cpp$"
        "/Add_Colormap.cpp$"
        "/Add_Texture_Atlas_Gpu.cpp$"
        "/Create_Blending_X_Resource.cpp$"
        "/Create_Binary_X_Resource.cpp$"
        "/Create_Square_Resource.cpp$"
        "/Create_Grid_Resource.cpp$"
        "/Save_Texture_Png.cpp$"
        "/Save_Texture_Array_Png.cpp$"
        "/Render_Resource_To_Textures.cpp$"
        "/Suppress_Texture_Warning.cpp$"
        "/Merge_Textures.cpp$"
        "/Hud_Tracker.cpp$"
        "/Minimap_Logic.cpp$"
        "/Parameter_Setter_Logic.cpp$"
        "/Countdown_Visual.cpp$"
        "/Key_Bindings_Logic.cpp$"
        "/Reload_Required.cpp$"
        "/Scene_Selector_Logic.cpp$"
        "/Players_Stats_Logic.cpp$"
        "/Tab_Menu_Logic.cpp$"
        "/Video_Mode_Selector_Logic.cpp$"
        "/Visual_Movable_Circular_Logger.cpp$"
        "/Visual_Movable_3rd_Logger.cpp$"
        "/Visual_Bullet_Count.cpp$"
        "/Visual_Movable_Text_Logger.cpp$"
        "/Renderable_Scenes.cpp$"
        "/Renderable_Scene.cpp$"
        "/Key_Bindings.cpp$"
        "/Hud_Opponent_Zoom_Logic.cpp$"
        "/Visual_Movable_Logger.cpp$"
        "/Clear_Requires_Reload.cpp$"
        "/Default_Persistent.cpp$"
        "/Persisted_Globals.cpp$"
        "/Clear_Selection_Ids.cpp$"
        "/Reload_Scene.cpp$"
        )
endif()

my_add_library(
    NAME MlibScene
    RECURSIVE
    ADDITIONAL_FILES ""
    EXCLUDED_FILES ${excluded_files}
)
# https://issuetracker.google.com/issues/214811726
if (NOT ANDROID)
    target_precompile_headers(MlibScene PRIVATE ../Misc/Precompiled_Header.hpp)
endif()

target_link_libraries(MlibScene
    PUBLIC
        MlibPhysics
        MlibPlayers
        MlibOpenGL
        MlibOsmLoader
        MlibSceneGraph
        MlibMacroExecutor
        MlibRemote)
if (OPENAL_FOUND)
    target_link_libraries(MlibScene
        PUBLIC
            MlibAudio)
endif()
