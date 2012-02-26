set(krunner_bing_SRCS
    imageiconengine.cpp
    bingjob.cpp
    bing.cpp
)

kde4_add_plugin(krunner_bing ${krunner_bing_SRCS})
target_link_libraries(krunner_bing ${KDE4_PLASMA_LIBS} ${KDE4_KIO_LIBS} ${QJSON_LIBRARIES})

install(TARGETS krunner_bing DESTINATION ${PLUGIN_INSTALL_DIR} )

install(FILES bing.desktop DESTINATION ${SERVICES_INSTALL_DIR})

