add_definitions(-DTRANSLATION_DOMAIN="plasma_runner_krunner_dictionary")

kcoreaddons_add_plugin(krunner_dictionary SOURCES ${dictionaryrunner_SRCS} INSTALL_NAMESPACE "kf${QT_MAJOR_VERSION}/krunner")
target_sources(krunner_dictionary PRIVATE dictionaryrunner.cpp)
target_link_libraries(krunner_dictionary Qt::Gui KF6::Runner KF6::I18n KF6::ConfigCore KF6::Notifications engine_dict_static)

kcoreaddons_add_plugin(kcm_krunner_dictionary INSTALL_NAMESPACE "kf${QT_MAJOR_VERSION}/krunner/kcms")
target_sources(kcm_krunner_dictionary PRIVATE dictionaryrunner_config.cpp)
target_link_libraries(kcm_krunner_dictionary KF6::Runner KF6::I18n KF6::KCMUtils)
