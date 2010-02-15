
add_subdirectory(plasmaweather)
add_subdirectory(rtm)
add_subdirectory(lancelot)
if(NOT WIN32)
add_subdirectory(lancelot-datamodels)
endif(NOT WIN32)

