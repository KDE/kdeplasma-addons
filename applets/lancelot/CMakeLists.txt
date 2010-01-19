find_package(KDE4 REQUIRED)

set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake ${CMAKE_MODULE_PATH})

SET(CMAKE_CXX_FLAGS_CODECHECK "${CMAKE_CXX_FLAGS} -save-temps")
SET(CMAKE_C_FLAGS_CODECHECK "${CMAKE_C_FLAGS} -save-temps")
message("Flags for CodeCheck are ${CMAKE_CXX_FLAGS_CODECHECK}")

add_subdirectory(tools)
add_subdirectory(libs)

add_subdirectory(app)
add_subdirectory(cmake)

