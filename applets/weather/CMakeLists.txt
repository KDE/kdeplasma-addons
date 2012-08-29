project(weatherapplet)

set(weatherapplet_SRCS
    weatherapplet.cpp)

kde4_add_plugin(plasma_applet_weather ${weatherapplet_SRCS})
target_link_libraries(plasma_applet_weather
    ${KDE4_PLASMA_LIBS}
    ${KDE4_KIO_LIBS}
    ${KDE4_KDEUI_LIBS}
    plasmaweather
    ${KDE4WORKSPACE_WEATHERION_LIBRARY}
    ${QT_QTDECLARATIVE_LIBRARY}
    ${KDE4_KUNITCONVERSION_LIBS})

install(TARGETS plasma_applet_weather DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES plasma-applet-weather.desktop DESTINATION ${SERVICES_INSTALL_DIR})
install(FILES wind-arrows.svgz DESTINATION ${DATA_INSTALL_DIR}/desktoptheme/default/weather/)
install(DIRECTORY package/ DESTINATION ${DATA_INSTALL_DIR}/plasma/packages/org.kde.weather)
