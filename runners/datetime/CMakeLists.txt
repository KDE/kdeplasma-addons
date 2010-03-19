project(datetimerunner)

set(krunner_datetime_SRCS
    datetimerunner.cpp
)

kde4_add_plugin(krunner_datetime ${krunner_datetime_SRCS})
target_link_libraries(krunner_datetime ${KDE4_PLASMA_LIBS})

install(TARGETS krunner_datetime DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES plasma-runner-datetime.desktop DESTINATION ${SERVICES_INSTALL_DIR})
