set(dictionaryrunner_SRCS dictionaryrunner.cpp dictionarymatchengine.cpp)
set(kcm_dictionaryrunner_SRCS dictionaryrunner_config.cpp)

kde4_add_plugin(krunner_dictionary ${dictionaryrunner_SRCS})
kde4_add_plugin(kcm_krunner_dictionary ${kcm_dictionaryrunner_SRCS})

target_link_libraries(krunner_dictionary ${KDE4_PLASMA_LIBS})
target_link_libraries(kcm_krunner_dictionary ${KDE4_KCMUTILS_LIBS})

install(TARGETS krunner_dictionary kcm_krunner_dictionary DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES plasma-runner-dictionary.desktop plasma-runner-dictionary_config.desktop DESTINATION ${SERVICES_INSTALL_DIR})
