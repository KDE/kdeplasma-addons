include_directories(${CMAKE_SOURCE_DIR} ${CMAKE_BINARY_DIR} ${KDE4_INCLUDES}
${QT_QTNETWORK_INCLUDES} ${QT_QTXML_INCLUDES} ${PLASMA_INCLUDE_DIR})

set(twitter_engine_SRCS
    imagesource.cpp
    timelinesource.cpp
    twitterengine.cpp
)

kde4_add_plugin(plasma_engine_twitter ${twitter_engine_SRCS})
target_link_libraries(plasma_engine_twitter ${KDE4_KIO_LIBS}
${KDE4_PLASMA_LIBS} ${QT_QTXML_LIBS} ${QT_QTNETWORK_LIBS})

install(TARGETS plasma_engine_twitter DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES plasma-dataengine-twitter.desktop DESTINATION ${SERVICES_INSTALL_DIR} )
install(FILES tweet.operations DESTINATION ${DATA_INSTALL_DIR}/plasma/services)

