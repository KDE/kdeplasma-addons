set(krunner_datetime_SRCS
    datetimerunner.cpp
)

add_library(krunner_datetime MODULE ${krunner_datetime_SRCS})
target_link_libraries(krunner_datetime
    KF5::Runner
    KF5::KIOWidgets
    KF5::I18n
    KF5::KDELibs4Support
)

install(TARGETS krunner_datetime DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES plasma-runner-datetime.desktop DESTINATION ${SERVICES_INSTALL_DIR})
