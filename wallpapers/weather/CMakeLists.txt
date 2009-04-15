project(plasma-wallpaper-weather)

set(weather_SRCS
    weatherwallpaper.cpp
    backgrounddelegate.cpp
    backgroundlistmodel.cpp
)

kde4_add_ui_files(weather_SRCS weatherAdvanced.ui)

kde4_add_plugin(plasma_wallpaper_weather ${weather_SRCS})
target_link_libraries(plasma_wallpaper_weather ${KDE4_PLASMA_LIBS} ${KDE4_KDECORE_LIBS} ${KDE4_KIO_LIBS} ${KDE4_KFILE_LIBS} ${KDE4_KNEWSTUFF2_LIBS} ${KDE4_THREADWEAVER_LIBRARY} plasmaweather)

install(TARGETS plasma_wallpaper_weather DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES plasma-wallpaper-weather.desktop DESTINATION ${SERVICES_INSTALL_DIR})
