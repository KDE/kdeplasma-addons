project(fuzzy-clock)

set(fuzzyclock_SRCS fuzzyClock.cpp)

kde4_add_ui_files(fuzzyclock_SRCS fuzzyClockConfig.ui )
kde4_add_plugin(plasma_applet_fuzzy_clock ${fuzzyclock_SRCS})
target_link_libraries(plasma_applet_fuzzy_clock ${KDE4WORKSPACE_PLASMACLOCK_LIBRARY} ${KDE4_PLASMA_LIBS} ${KDE4_KDEUI_LIBS})

install(TARGETS plasma_applet_fuzzy_clock DESTINATION ${PLUGIN_INSTALL_DIR})

install(FILES plasma-clock-fuzzy.desktop DESTINATION ${SERVICES_INSTALL_DIR})
