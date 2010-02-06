find_package(KDE4 REQUIRED)

set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake ${CMAKE_MODULE_PATH})

SET(CMAKE_CXX_FLAGS_CODECHECK "${CMAKE_CXX_FLAGS} -save-temps")
SET(CMAKE_C_FLAGS_CODECHECK "${CMAKE_C_FLAGS} -save-temps")
message("Flags for CodeCheck are ${CMAKE_CXX_FLAGS_CODECHECK}")

find_package(SharedMimeInfo REQUIRED)

# Finding liblancelot
find_package(Lancelot REQUIRED)
find_package(Lancelot-Datamodels REQUIRED)

add_subdirectory(tools)

add_subdirectory(application)
add_subdirectory(launcher)
add_subdirectory(parts)

add_subdirectory(themes)
add_subdirectory(icons)
add_subdirectory(cmake)

