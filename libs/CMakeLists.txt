
add_subdirectory(plasmaweather)
add_subdirectory(rtm)
add_subdirectory(lancelot)

if(NOT WIN32 AND KDE4WORKSPACE_FOUND)
   add_subdirectory(lancelot-datamodels)
endif(NOT WIN32 AND KDE4WORKSPACE_FOUND)

