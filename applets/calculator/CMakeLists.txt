project(plasma-calculator)

set(calculator_SRCS
    calculator.cpp)

kde4_add_plugin(plasma_applet_calculator ${calculator_SRCS})
target_link_libraries(plasma_applet_calculator ${KDE4_KDEUI_LIBS} ${KDE4_PLASMA_LIBS})

install(TARGETS plasma_applet_calculator DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES plasma-applet-calculator.desktop DESTINATION ${SERVICES_INSTALL_DIR})
