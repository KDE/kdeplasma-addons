# cmake_minimum_required(VERSION 2.6)
find_package(KDE4 REQUIRED)
KDE4_NO_ENABLE_FINAL(lancelot)
set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake ${CMAKE_MODULE_PATH})

# Profiling build type
# SET(CMAKE_BUILD_TYPE Profile)

# SET(CMAKE_CXX_FLAGS_PROFILE ${CMAKE_CXX_FLAGS} -pg)
# SET(CMAKE_C_FLAGS_PROFILE ${CMAKE_C_FLAGS} -pg)

SET(CMAKE_CXX_FLAGS_CODECHECK "${CMAKE_CXX_FLAGS} -save-temps")
SET(CMAKE_C_FLAGS_CODECHECK "${CMAKE_C_FLAGS} -save-temps")
message("Flags for CodeCheck are ${CMAKE_CXX_FLAGS_CODECHECK}")

#add_definitions(-Werror=overloaded-virtual)

add_subdirectory(tools)
add_subdirectory(libs)

add_subdirectory(app)
# add_subdirectory(cmake)

