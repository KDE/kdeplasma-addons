add_definitions(-DTRANSLATION_DOMAIN="plasma_engine_konsoleprofiles")

set(konsoleprofilesengine_SRCS
    konsoleprofilesengine.cpp
    konsoleprofilesservice.cpp
)

kcoreaddons_add_plugin(plasma_engine_konsoleprofiles SOURCES ${konsoleprofilesengine_SRCS} INSTALL_NAMESPACE "plasma/dataengine")
target_link_libraries(plasma_engine_konsoleprofiles
    KF5::Plasma
    KF5::Notifications
    KF5::KIOGui
    KF5::Service
)

install(FILES org.kde.plasma.dataengine.konsoleprofiles.operations DESTINATION ${PLASMA_DATA_INSTALL_DIR}/services)
