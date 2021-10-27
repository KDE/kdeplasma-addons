add_definitions(-DTRANSLATION_DOMAIN="plasma_runner_CharacterRunner")

kcoreaddons_add_plugin(krunner_charrunner SOURCES charrunner.cpp INSTALL_NAMESPACE "kf5/krunner")
target_link_libraries(krunner_charrunner
    KF5::Runner
    KF5::I18n
)

kcoreaddons_add_plugin(kcm_krunner_charrunner INSTALL_NAMESPACE "kf5/krunner/kcms")
target_sources(kcm_krunner_charrunner PRIVATE charrunner_config.cpp)
ki18n_wrap_ui(kcm_krunner_charrunner charrunner_config.ui)
target_link_libraries(kcm_krunner_charrunner
    KF5::Runner
    KF5::KCMUtils
    KF5::I18n
)
