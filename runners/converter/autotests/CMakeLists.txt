remove_definitions(-DQT_NO_CAST_FROM_ASCII)

include(ECMAddTests)

ecm_add_test(converterrunnertest.cpp TEST_NAME converterrunnertest LINK_LIBRARIES Qt::Gui Qt::Test KF5::Runner KF5::UnitConversion)
krunner_configure_test(converterrunnertest unitconverter)
