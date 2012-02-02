
add_subdirectory(plasmaweather)
add_subdirectory(rtm)

if(${KDE_PLATFORM_PROFILE} STREQUAL "Desktop")
    add_subdirectory(lancelot)

if(NOT WIN32 AND KDE4WORKSPACE_FOUND)
   add_subdirectory(lancelot-datamodels)
endif(NOT WIN32 AND KDE4WORKSPACE_FOUND)

endif(${KDE_PLATFORM_PROFILE} STREQUAL "Desktop")
