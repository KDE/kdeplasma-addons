set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

# Profiling build type
# SET(CMAKE_BUILD_TYPE Profile)
# SET(CMAKE_CXX_FLAGS_PROFILE ${CMAKE_CXX_FLAGS} -pg)
# SET(CMAKE_C_FLAGS_PROFILE ${CMAKE_C_FLAGS} -pg)



#add_definitions(-Werror=overloaded-virtual)

add_subdirectory(tools)
add_subdirectory(libs)

add_subdirectory(app)
add_subdirectory(cmake)

