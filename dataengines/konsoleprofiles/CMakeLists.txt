add_definitions(-DTRANSLATION_DOMAIN="plasma_engine_konsoleprofiles")

set(konsoleprofilesengine_SRCS
    konsoleprofilesengine.cpp
    konsoleprofilesservice.cpp
)

add_library(plasma_engine_konsoleprofiles MODULE ${konsoleprofilesengine_SRCS})
target_link_libraries(plasma_engine_konsoleprofiles
    KF5::Plasma
    KF5::Notifications
    KF5::KIOGui
    KF5::Service
)

kcoreaddons_desktop_to_json(plasma_engine_konsoleprofiles plasma-dataengine-konsoleprofiles.desktop SERVICE_TYPES plasma-dataengine.desktop)

install(TARGETS plasma_engine_konsoleprofiles DESTINATION ${KDE_INSTALL_PLUGINDIR}/plasma/dataengine)
install(FILES plasma-dataengine-konsoleprofiles.desktop DESTINATION ${KDE_INSTALL_KSERVICES5DIR} )
install(FILES org.kde.plasma.dataengine.konsoleprofiles.operations DESTINATION ${PLASMA_DATA_INSTALL_DIR}/services)

