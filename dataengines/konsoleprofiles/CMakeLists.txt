set(konsoleprofilesengine_SRCS
    konsoleprofilesengine.cpp
    konsoleprofilesservice.cpp
)

kservice_desktop_to_json(plasma-dataengine-konsoleprofiles.desktop)

add_library(plasma_engine_konsoleprofiles ${konsoleprofilesengine_SRCS})
target_link_libraries(plasma_engine_konsoleprofiles
    KF5::Plasma
    KF5::Service
    KF5::KIOCore
    KF5::KDE4Support
)

install(TARGETS plasma_engine_konsoleprofiles DESTINATION ${PLUGIN_INSTALL_DIR}/plasma/dataengine)
install(FILES plasma-dataengine-konsoleprofiles.desktop DESTINATION ${SERVICES_INSTALL_DIR} )
install(FILES org.kde.plasma.dataengine.konsoleprofiles.operations DESTINATION ${PLASMA_DATA_INSTALL_DIR}/services)

