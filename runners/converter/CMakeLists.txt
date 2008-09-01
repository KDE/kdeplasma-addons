set(krunner_converterrunner_SRCS
    converterrunner.cpp
    length.cpp
    area.cpp
    volume.cpp
    temperature.cpp
    simpleunit.cpp
    speed.cpp
    mass.cpp
)

kde4_add_plugin(krunner_converterrunner ${krunner_converterrunner_SRCS})
target_link_libraries(krunner_converterrunner ${KDE4_KIO_LIBS} ${PLASMA_LIBS})

install(TARGETS krunner_converterrunner DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES plasma-runner-converter.desktop DESTINATION ${SERVICES_INSTALL_DIR})

