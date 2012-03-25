include_directories(${QJSON_INCLUDE_DIR})

set(krunner_duckduckgo_SRCS
    duckduckgo.cpp
)

kde4_add_plugin(krunner_duckduckgo ${krunner_duckduckgo_SRCS})
target_link_libraries(krunner_duckduckgo ${KDE4_PLASMA_LIBS} ${KDE4_KIO_LIBS} ${QJSON_LIBRARIES})

install(TARGETS krunner_duckduckgo DESTINATION ${PLUGIN_INSTALL_DIR} )

install(FILES duckduckgo.desktop DESTINATION ${SERVICES_INSTALL_DIR})

