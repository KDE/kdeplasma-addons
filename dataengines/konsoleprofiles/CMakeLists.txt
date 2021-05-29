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

kcoreaddons_desktop_to_json(plasma_engine_konsoleprofiles plasma-dataengine-konsoleprofiles.desktop SERVICE_TYPES plasma-dataengine.desktop)

install(FILES plasma-dataengine-konsoleprofiles.desktop DESTINATION ${KDE_INSTALL_KSERVICES5DIR} )
install(FILES org.kde.plasma.dataengine.konsoleprofiles.operations DESTINATION ${PLASMA_DATA_INSTALL_DIR}/services)

