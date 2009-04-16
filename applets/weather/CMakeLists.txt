project(weatherapplet)

set(weatherapplet_SRCS
    weatherapplet.cpp
    weatherview.cpp
    weatherdelegate.cpp)

kde4_add_plugin(plasma_applet_weather ${weatherapplet_SRCS})
target_link_libraries(plasma_applet_weather ${KDE4_PLASMA_LIBS} ${KDE4_KIO_LIBS} ${KDE4_KDEUI_LIBS} plasmaweather weather_ion ${QT_QTWEBKIT_LIBRARY})

install(TARGETS plasma_applet_weather DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES plasma-applet-weather.desktop DESTINATION ${SERVICES_INSTALL_DIR})

install(FILES 
        wind-arrows.svgz 
        DESTINATION ${DATA_INSTALL_DIR}/desktoptheme/default/weather/)
