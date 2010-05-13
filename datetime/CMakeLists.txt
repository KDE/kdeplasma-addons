project(datetimerunner)

set(plasma_runner_datetime_SRCS
    datetimerunner.cpp
)

kde4_add_plugin(plasma_runner_datetime ${plasma_runner_datetime_SRCS})
target_link_libraries(plasma_runner_datetime ${KDE4_PLASMA_LIBS})

install(TARGETS plasma_runner_datetime DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES plasma-runner-datetime.desktop DESTINATION ${SERVICES_INSTALL_DIR})
