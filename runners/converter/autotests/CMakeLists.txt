remove_definitions(-DQT_NO_CAST_FROM_ASCII)

include(ECMAddTests)

ecm_add_test(converterrunnertest.cpp TEST_NAME converterrunnertest LINK_LIBRARIES Qt5::Test krunner_converter_static)
