project(libplasmaconverter)

set(krunner_converter_SRCS
    converterrunner.cpp
)

set(plasmaconverter_LIB_SRCS
    converter.cpp
    unit.cpp
    simpleunit.cpp
    length.cpp
    area.cpp
    volume.cpp
    temperature.cpp
    speed.cpp
    mass.cpp
)

set(plasmaconverter_HEADERS
    converter.h
    unit.h
)

kde4_add_library(plasmaconverter SHARED ${plasmaconverter_LIB_SRCS})
target_link_libraries(plasmaconverter ${KDE4_KDECORE_LIBS})
set_target_properties(plasmaconverter PROPERTIES VERSION ${GENERIC_LIB_VERSION} SOVERSION ${GENERIC_LIB_SOVERSION})
install(TARGETS plasmaconverter DESTINATION ${LIB_INSTALL_DIR})
install(FILES ${plasmaconverter_HEADERS} DESTINATION ${INCLUDE_INSTALL_DIR}/ COMPONENT Devel)

kde4_add_plugin(krunner_converter ${krunner_converter_SRCS})
target_link_libraries(krunner_converter plasmaconverter ${KDE4_KIO_LIBS} ${PLASMA_LIBS})
install(TARGETS krunner_converter DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES plasma-runner-converter.desktop DESTINATION ${SERVICES_INSTALL_DIR})

