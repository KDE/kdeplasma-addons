add_definitions(-DTRANSLATION_DOMAIN="plasma_runner_spellcheckrunner")

set(krunner_spellcheckrunner_SRCS
    spellcheck.cpp
)

set(kcm_krunner_spellcheck_SRCS
    spellcheck_config.cpp
)

ki18n_wrap_ui(kcm_krunner_spellcheck_SRCS spellcheck_config.ui)
add_library(kcm_krunner_spellcheck MODULE ${kcm_krunner_spellcheck_SRCS})
target_link_libraries(kcm_krunner_spellcheck 
    Qt5::Gui
    KF5::Runner
    KF5::KCMUtils
    KF5::KDELibs4Support
    KF5::I18n
)

add_library(krunner_spellcheck MODULE ${krunner_spellcheckrunner_SRCS})
target_link_libraries(krunner_spellcheck
    KF5::Runner
    KF5::KIOWidgets
    KF5::I18n
    KF5::SonnetCore
)

install(TARGETS krunner_spellcheck kcm_krunner_spellcheck
        DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES plasma-runner-spellchecker.desktop plasma-runner-spellchecker_config.desktop
        DESTINATION ${SERVICES_INSTALL_DIR})
