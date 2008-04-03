set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

# IF(CMAKE_COMPILER_IS_GNUCC)
#   SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fmessage-length=0")
# ENDIF(CMAKE_COMPILER_IS_GNUCC)
# IF(CMAKE_COMPILER_IS_GNUCXX)
#   SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fmessage-length=0")
# ENDIF(CMAKE_COMPILER_IS_GNUCXX)

add_subdirectory(tools)
add_subdirectory(libs)

add_subdirectory(app)
add_subdirectory(pics)
add_subdirectory(cmake)

