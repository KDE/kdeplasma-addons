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
    KF5::I18n
)

add_library(krunner_spellcheck MODULE ${krunner_spellcheckrunner_SRCS})
kcoreaddons_desktop_to_json(krunner_spellcheck plasma-runner-spellchecker.desktop)
target_link_libraries(krunner_spellcheck
    KF5::Runner
    KF5::KIOWidgets
    KF5::I18n
    KF5::SonnetCore
)

install(TARGETS krunner_spellcheck
        DESTINATION ${KDE_INSTALL_PLUGINDIR}/kf5/krunner)
install(TARGETS kcm_krunner_spellcheck
        DESTINATION ${KDE_INSTALL_PLUGINDIR})
install(FILES plasma-runner-spellchecker_config.desktop
        DESTINATION ${KDE_INSTALL_KSERVICES5DIR})
