add_definitions(-DTRANSLATION_DOMAIN="plasma_runner_krunner_dictionary")

set(dictionaryrunner_SRCS dictionaryrunner.cpp dictionarymatchengine.cpp)
set(kcm_dictionaryrunner_SRCS dictionaryrunner_config.cpp)

add_library(krunner_dictionary MODULE ${dictionaryrunner_SRCS})
kcoreaddons_desktop_to_json(krunner_dictionary plasma-runner-dictionary.desktop )
target_link_libraries(krunner_dictionary KF5::Runner KF5::I18n)

add_library(kcm_krunner_dictionary MODULE ${kcm_dictionaryrunner_SRCS})
target_link_libraries(kcm_krunner_dictionary KF5::Runner KF5::I18n KF5::KCMUtils)

install(TARGETS krunner_dictionary DESTINATION ${KDE_INSTALL_PLUGINDIR}/kf5/krunner)
install(TARGETS kcm_krunner_dictionary DESTINATION ${KDE_INSTALL_PLUGINDIR})
install(FILES plasma-runner-dictionary_config.desktop DESTINATION ${KDE_INSTALL_KSERVICES5DIR})
