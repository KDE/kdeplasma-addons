project(plasma-wallpaper-weather)

# Find the required Libaries
find_package(KDE4 REQUIRED)
include(KDE4Defaults)

add_definitions (${QT_DEFINITIONS} ${KDE4_DEFINITIONS})
include_directories(
   ${CMAKE_SOURCE_DIR}
   ${CMAKE_BINARY_DIR}
   ${KDE4_INCLUDES}
)

set(weather_SRCS
    weather.cpp
    backgroundpackage.cpp
)

kde4_add_ui_files(weather_SRCS weatherAddPlace.ui)
kde4_add_ui_files(weather_SRCS weatherLocations.ui)

kde4_add_plugin(plasma_wallpaper_weather ${weather_SRCS})
target_link_libraries(plasma_wallpaper_weather ${KDE4_PLASMA_LIBS} ${KDE4_KDECORE_LIBS} ${KDE4_KIO_LIBS} ${KDE4_THREADWEAVER_LIBRARY})

install(TARGETS plasma_wallpaper_weather DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES plasma-wallpaper-weather.desktop DESTINATION ${SERVICES_INSTALL_DIR})


FILE(GLOB weather_jpg wallpapers/*.jpg)

install(DIRECTORY wallpapers/At_Night DESTINATION ${WALLPAPER_INSTALL_DIR} PATTERN .svn EXCLUDE)
install(DIRECTORY wallpapers/Blue_Sun DESTINATION ${WALLPAPER_INSTALL_DIR} PATTERN .svn EXCLUDE)
install(DIRECTORY wallpapers/Hail DESTINATION ${WALLPAPER_INSTALL_DIR} PATTERN .svn EXCLUDE)
install(DIRECTORY wallpapers/Icy_Tree DESTINATION ${WALLPAPER_INSTALL_DIR} PATTERN .svn EXCLUDE)
install(DIRECTORY wallpapers/JK_Bridge_at_Night DESTINATION ${WALLPAPER_INSTALL_DIR} PATTERN .svn EXCLUDE)
install(DIRECTORY wallpapers/Lightning DESTINATION ${WALLPAPER_INSTALL_DIR} PATTERN .svn EXCLUDE)
install(DIRECTORY wallpapers/Winter_Track DESTINATION ${WALLPAPER_INSTALL_DIR} PATTERN .svn EXCLUDE)
