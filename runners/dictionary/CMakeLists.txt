add_definitions(-DTRANSLATION_DOMAIN="plasma_runner_krunner_dictionary")

set(dictionaryrunner_SRCS dictionaryrunner.cpp dictionarymatchengine.cpp)
set(kcm_dictionaryrunner_SRCS dictionaryrunner_config.cpp)

add_library(krunner_dictionary MODULE ${dictionaryrunner_SRCS})
add_library(kcm_krunner_dictionary MODULE ${kcm_dictionaryrunner_SRCS})

target_link_libraries(krunner_dictionary KF5::Runner KF5::I18n)
target_link_libraries(kcm_krunner_dictionary KF5::Runner KF5::I18n KF5::KCMUtils)

install(TARGETS krunner_dictionary kcm_krunner_dictionary DESTINATION ${KDE_INSTALL_PLUGINDIR})
install(FILES plasma-runner-dictionary.desktop plasma-runner-dictionary_config.desktop DESTINATION ${KDE_INSTALL_KSERVICES5DIR})
