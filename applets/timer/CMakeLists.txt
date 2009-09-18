
# Project needs a name of course
project(timer)

# We add our source code here
set(timer_SRCS timer.cpp timerdigit.cpp customtimeeditor.cpp)
kde4_add_ui_files(timer_SRCS timerConfig.ui predefinedTimersConfig.ui)

# Now make sure all files get to the right place
kde4_add_plugin(plasma_applet_timer ${timer_SRCS})
target_link_libraries(plasma_applet_timer
                      ${KDE4_PLASMA_LIBS} ${KDE4_KDEUI_LIBS})

install(TARGETS plasma_applet_timer
        DESTINATION ${PLUGIN_INSTALL_DIR})

install(FILES plasma-applet-timer.desktop
        DESTINATION ${SERVICES_INSTALL_DIR})

install(FILES timer.svgz
	DESTINATION ${DATA_INSTALL_DIR}/desktoptheme/default/widgets/)
