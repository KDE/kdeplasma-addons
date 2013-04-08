include_directories(${QJSON_INCLUDE_DIR})

set(krunner_youtube_SRCS
    imageiconengine.cpp
    youtube.cpp
    tubejob.cpp
)

add_subdirectory(icons)

kde4_add_plugin(krunner_youtube ${krunner_youtube_SRCS})
target_link_libraries(krunner_youtube
    ${KDE4_PLASMA_LIBS}
    ${KDE4_KIO_LIBS}
    ${QJSON_LIBRARIES}
    ${qjson_LIBRARIES}
    ${KDE4_SOLID_LIBS})

install(TARGETS krunner_youtube DESTINATION ${PLUGIN_INSTALL_DIR} )

install(FILES plasma-runner-youtube.desktop DESTINATION ${SERVICES_INSTALL_DIR})
