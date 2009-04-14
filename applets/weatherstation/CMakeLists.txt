project(plasma-weatherstation)

set(weatherstation_SRCS
    weatherstation.cpp
    lcd.cpp
)

kde4_add_ui_files(weatherstation_SRCS
    appearanceconfig.ui
)

kde4_add_plugin(plasma_applet_weatherstation ${weatherstation_SRCS})
target_link_libraries(plasma_applet_weatherstation conversion plasmaweather
    ${KDE4_KDEUI_LIBS} ${KDE4_KIO_LIBS} ${KDE4_PLASMA_LIBS} ${KDE4WORKSPACE_WEATHERION_LIBRARY} )

install(TARGETS plasma_applet_weatherstation DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES plasma-applet-weatherstation.desktop
        DESTINATION ${SERVICES_INSTALL_DIR})

install(FILES
    lcd.svgz
    lcd_panel.svgz
    DESTINATION ${DATA_INSTALL_DIR}/desktoptheme/default/weatherstation/)
