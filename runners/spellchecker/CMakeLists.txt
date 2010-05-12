set(krunner_spellcheckrunner_SRCS
    spellcheck.cpp
)

set(kcm_krunner_spellcheck_SRCS
    spellcheck_config.cpp
)

kde4_add_ui_files(kcm_krunner_spellcheck_SRCS spellcheck_config.ui)
kde4_add_plugin(kcm_krunner_spellcheck ${kcm_krunner_spellcheck_SRCS})
target_link_libraries(kcm_krunner_spellcheck 
    ${KDE4_KDECORE_LIBS}
    ${KDE4_KDEUI_LIBS}
    ${KDE4_KCMUTILS_LIBS}
    ${QT_QTCORE_LIBRARY}
    ${QT_QTGUI_LIBRARY}
    )

kde4_add_plugin(krunner_spellcheckrunner ${krunner_spellcheckrunner_SRCS})
target_link_libraries(krunner_spellcheckrunner ${KDE4_KCMUTILS_LIBS} ${KDE4_KDEUI_LIBS} ${KDE4_PLASMA_LIBS})

install(TARGETS krunner_spellcheckrunner kcm_krunner_spellcheck DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES plasma-runner-spellchecker.desktop plasma-runner-spellchecker_config.desktop DESTINATION ${SERVICES_INSTALL_DIR})

