add_definitions(-DTRANSLATION_DOMAIN="plasma_runner_spellcheckrunner")

add_library(krunner_spellcheck_static STATIC spellcheck.cpp)
target_link_libraries(krunner_spellcheck_static
    KF5::Runner
    KF5::KIOWidgets
    KF5::I18n
    KF5::SonnetCore
)

kcoreaddons_add_plugin(krunner_spellcheck SOURCES spellcheck.cpp INSTALL_NAMESPACE "kf${QT_MAJOR_VERSION}/krunner")
target_link_libraries(krunner_spellcheck
    krunner_spellcheck_static
)

kcoreaddons_add_plugin(kcm_krunner_spellcheck INSTALL_NAMESPACE "kf${QT_MAJOR_VERSION}/krunner/kcms")
ki18n_wrap_ui(kcm_krunner_spellcheck spellcheck_config.ui)
target_sources(kcm_krunner_spellcheck PRIVATE spellcheck_config.cpp)
target_link_libraries(kcm_krunner_spellcheck 
    Qt::Gui
    KF5::Runner
    KF5::KCMUtils
    KF5::I18n
)

if(BUILD_TESTING)
   add_subdirectory(autotests)
endif()
