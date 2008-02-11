set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
message(" 1 #############################")

include(PlasmaUIMacros)
KDE4_ADD_PLASMA_UI_FILES(srcs "./ui/asd.xml")
message(" 1 #############################")


add_subdirectory(app)
add_subdirectory(pics)
add_subdirectory(cmake)

