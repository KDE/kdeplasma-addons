add_definitions(-DTRANSLATION_DOMAIN="plasma_runner_krunner_dictionary")

kcoreaddons_add_plugin(krunner_dictionary SOURCES ${dictionaryrunner_SRCS} INSTALL_NAMESPACE "kf6/krunner")
target_sources(krunner_dictionary PRIVATE dictionaryrunner.cpp)
target_link_libraries(krunner_dictionary Qt::Gui KF6::Runner KF6::I18n KF6::ConfigCore KF6::Notifications engine_dict_static)

kcoreaddons_add_plugin(kcm_krunner_dictionary INSTALL_NAMESPACE "kf6/krunner/kcms")
target_compile_definitions(kcm_krunner_dictionary PRIVATE -DKRUNNER_PLUGIN_NAME="krunner_dictionary")
target_sources(kcm_krunner_dictionary PRIVATE dictionaryrunner_config.cpp)
target_link_libraries(kcm_krunner_dictionary KF6::Runner KF6::I18n KF6::KCMUtils)

# kcmshell6 < 6.27.0 doesn't know the --smoke-test param, so skip this test for
# now.
# TODO: Drop this when KF6_MIN_VERSION >= 6.27.0
if(KF6KCMUtils_VERSION VERSION_GREATER_EQUAL "6.27.0")
    add_test(NAME kcm_krunner_dictionary_smoketest COMMAND kcmshell6 --smoke-test kf6/krunner/kcms/kcm_krunner_dictionary)
    set_tests_properties(kcm_krunner_dictionary_smoketest PROPERTIES
        ENVIRONMENT_MODIFICATION QT_PLUGIN_PATH=path_list_prepend:${CMAKE_BINARY_DIR}/bin
    )
endif()
