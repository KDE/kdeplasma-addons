include_directories(${QJSON_INCLUDE_DIR} ${qjson_INCLUDE_DIR})

set (krunner_translator_SRCS
    translator.cpp
    translatorjob.cpp
)

kde4_add_plugin(krunner_translator ${krunner_translator_SRCS})
target_link_libraries(krunner_translator ${KDE4_KUTILS_LIBS} ${KDE4_KDEUI_LIBS} ${KDE4_PLASMA_LIBS} ${KDE4_KIO_LIBS} ${QJSON_LIBRARIES} ${qjson_LIBRARIES})

install(TARGETS krunner_translator DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES plasma-runner-translator.desktop DESTINATION ${SERVICES_INSTALL_DIR})