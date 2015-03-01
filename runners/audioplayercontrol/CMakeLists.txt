add_definitions(-DTRANSLATION_DOMAIN="plasma_runner_audioplayercontrol")

set(krunner_audioplayercontrol_SRCS
    imageiconengine.cpp
    audioplayercontrolrunner.cpp
)

set(kcm_krunner_audioplayercontrol_SRCS
    audioplayercontrolrunner_config.cpp
)

ki18n_wrap_ui(kcm_krunner_audioplayercontrol_SRCS audioplayercontrolrunner_config.ui)
add_library(kcm_krunner_audioplayercontrol MODULE ${kcm_krunner_audioplayercontrol_SRCS})
target_link_libraries(kcm_krunner_audioplayercontrol
    Qt5::Gui
    KF5::Runner
    KF5::KCMUtils
    KF5::KDELibs4Support
    KF5::I18n
)

add_library(krunner_audioplayercontrol MODULE ${krunner_audioplayercontrol_SRCS})
target_link_libraries(krunner_audioplayercontrol
    KF5::Runner
    KF5::KIOWidgets
    KF5::I18n
    KF5::KDELibs4Support
)


install(TARGETS krunner_audioplayercontrol kcm_krunner_audioplayercontrol
        DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES plasma-runner-audioplayercontrol.desktop plasma-runner-audioplayercontrol_config.desktop
        DESTINATION ${SERVICES_INSTALL_DIR})
