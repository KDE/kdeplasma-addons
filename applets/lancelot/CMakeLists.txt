set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

add_definitions(-Werror=overloaded-virtual)

add_subdirectory(tools)
add_subdirectory(libs)

add_subdirectory(app)
add_subdirectory(pics)
add_subdirectory(cmake)

