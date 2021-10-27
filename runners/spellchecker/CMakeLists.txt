add_definitions(-DTRANSLATION_DOMAIN="plasma_runner_spellcheckrunner")

kcoreaddons_add_plugin(kcm_krunner_spellcheck INSTALL_NAMESPACE "kf5/krunner/kcms")
ki18n_wrap_ui(kcm_krunner_spellcheck spellcheck_config.ui)
target_sources(kcm_krunner_spellcheck PRIVATE spellcheck_config.cpp)
target_link_libraries(kcm_krunner_spellcheck 
    Qt::Gui
    KF5::Runner
    KF5::KCMUtils
    KF5::I18n
)

kcoreaddons_add_plugin(krunner_spellcheck SOURCES spellcheck.cpp INSTALL_NAMESPACE "kf5/krunner")
target_link_libraries(krunner_spellcheck
    KF5::Runner
    KF5::KIOWidgets
    KF5::I18n
    KF5::SonnetCore
)
