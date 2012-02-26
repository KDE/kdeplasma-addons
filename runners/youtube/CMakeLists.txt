set(krunner_youtube_SRCS
    youtube.cpp
    tubejob.cpp
)

kde4_add_plugin(krunner_youtube ${krunner_youtube_SRCS})
target_link_libraries(krunner_youtube ${KDE4_PLASMA_LIBS} ${KDE4_KIO_LIBS} ${QJSON_LIBRARIES})

install(TARGETS krunner_youtube DESTINATION ${PLUGIN_INSTALL_DIR} )

install(FILES youtube.desktop DESTINATION ${SERVICES_INSTALL_DIR})

