project(plasma-wallpaper-pattern)

find_package(QImageBlitz REQUIRED)

include_directories(${QIMAGEBLITZ_INCLUDES})

set(pattern_SRCS
    pattern.cpp
)

kde4_add_ui_files(pattern_SRCS config.ui)

kde4_add_plugin(plasma_wallpaper_pattern ${pattern_SRCS})
target_link_libraries(plasma_wallpaper_pattern ${QIMAGEBLITZ_LIBRARIES} ${KDE4_PLASMA_LIBS} ${KDE4_KDECORE_LIBS})

install(TARGETS plasma_wallpaper_pattern DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES plasma-wallpaper-pattern.desktop DESTINATION ${SERVICES_INSTALL_DIR})


FILE(GLOB pattern_desktop patterns/*.desktop)
FILE(GLOB pattern_png patterns/*.jpg)
FILE(GLOB pattern_jpg patterns/*.png)

INSTALL(FILES ${pattern_desktop}  ${pattern_png} ${pattern_jpg} DESTINATION ${DATA_INSTALL_DIR}/plasma_wallpaper_pattern/patterns)
