set(potd_engine_SRCS
	cachedprovider.cpp
	potd.cpp
)

kcoreaddons_add_plugin(plasma_engine_potd SOURCES ${potd_engine_SRCS} INSTALL_NAMESPACE "plasma/dataengine")
target_link_libraries(plasma_engine_potd plasmapotdprovidercore
    KF5::Plasma
    KF5::KIOCore
)

########### plugin core library ############
set(POTDPROVIDER_VERSION 1.0.0)
set(POTDPROVIDER_VERSION_MAJOR 1)

set(potd_provider_core_SRCS
	potdprovider.cpp
	${CMAKE_CURRENT_BINARY_DIR}/plasma_potd_export.h
)

add_library( plasmapotdprovidercore SHARED ${potd_provider_core_SRCS} )
add_library(Plasma::PotdProvider ALIAS plasmapotdprovidercore)
set_target_properties(plasmapotdprovidercore PROPERTIES
    VERSION ${POTDPROVIDER_VERSION}
    SOVERSION ${POTDPROVIDER_VERSION_MAJOR}
    EXPORT_NAME PotdProvider
)
target_link_libraries( plasmapotdprovidercore Qt::Gui KF5::CoreAddons KF5::ConfigCore KF5::KIOCore)
target_include_directories(plasmapotdprovidercore
    PUBLIC "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>"
    INTERFACE "$<INSTALL_INTERFACE:${KDE_INSTALL_INCLUDEDIR}>"
)
generate_export_header(plasmapotdprovidercore BASE_NAME PLASMA_POTD EXPORT_FILE_NAME plasma_potd_export.h)

install(TARGETS plasmapotdprovidercore EXPORT plasmapotdproviderTargets ${KDE_INSTALL_TARGETS_DEFAULT_ARGS} )
install(FILES
        potdprovider.h
        ${CMAKE_CURRENT_BINARY_DIR}/plasma_potd_export.h
    DESTINATION ${KDE_INSTALL_INCLUDEDIR}/plasma/potdprovider
    COMPONENT Devel
)

write_basic_package_version_file(${CMAKE_CURRENT_BINARY_DIR}/PlasmaPotdProviderConfigVersion.cmake
    VERSION "${POTDPROVIDER_VERSION}"
    COMPATIBILITY SameMajorVersion
)

set(CMAKECONFIG_INSTALL_DIR ${KDE_INSTALL_LIBDIR}/cmake/PlasmaPotdProvider)
configure_package_config_file(PlasmaPotdProvider.cmake.in
        "${CMAKE_CURRENT_BINARY_DIR}/PlasmaPotdProviderConfig.cmake"
    INSTALL_DESTINATION ${CMAKECONFIG_INSTALL_DIR}
)

install(FILES
        ${CMAKE_CURRENT_BINARY_DIR}/PlasmaPotdProviderConfig.cmake
        ${CMAKE_CURRENT_BINARY_DIR}/PlasmaPotdProviderConfigVersion.cmake
    DESTINATION ${CMAKECONFIG_INSTALL_DIR}
    COMPONENT Devel
)

install(EXPORT plasmapotdproviderTargets
    NAMESPACE Plasma::
    DESTINATION ${CMAKECONFIG_INSTALL_DIR}
    FILE PlasmaPotdProviderTargets.cmake
    COMPONENT Devel
)

########### plugins ###############

kcoreaddons_add_plugin(plasma_potd_flickrprovider SOURCES flickrprovider.cpp INSTALL_NAMESPACE "potd")
target_link_libraries( plasma_potd_flickrprovider plasmapotdprovidercore KF5::KIOCore KF5::CoreAddons)

kcoreaddons_add_plugin(plasma_potd_apodprovider SOURCES apodprovider.cpp INSTALL_NAMESPACE "potd")
target_link_libraries( plasma_potd_apodprovider plasmapotdprovidercore KF5::KIOCore)

kcoreaddons_add_plugin(plasma_potd_natgeoprovider SOURCES natgeoprovider.cpp INSTALL_NAMESPACE "potd")
target_link_libraries( plasma_potd_natgeoprovider plasmapotdprovidercore KF5::KIOCore)

kcoreaddons_add_plugin(plasma_potd_epodprovider SOURCES epodprovider.cpp INSTALL_NAMESPACE "potd")
target_link_libraries( plasma_potd_epodprovider plasmapotdprovidercore KF5::KIOCore)

kcoreaddons_add_plugin(plasma_potd_noaaprovider SOURCES noaaprovider.cpp INSTALL_NAMESPACE "potd")
target_link_libraries( plasma_potd_noaaprovider plasmapotdprovidercore KF5::KIOCore)

kcoreaddons_add_plugin(plasma_potd_wcpotdprovider SOURCES wcpotdprovider.cpp INSTALL_NAMESPACE "potd")
target_link_libraries( plasma_potd_wcpotdprovider plasmapotdprovidercore KF5::KIOCore )

kcoreaddons_add_plugin(plasma_potd_bingprovider SOURCES bingprovider.cpp INSTALL_NAMESPACE "potd")
target_link_libraries( plasma_potd_bingprovider plasmapotdprovidercore KF5::KIOCore )

kcoreaddons_add_plugin(plasma_potd_unsplashprovider SOURCES unsplashprovider.cpp INSTALL_NAMESPACE "potd")
target_link_libraries( plasma_potd_unsplashprovider plasmapotdprovidercore KF5::KIOCore )

kcoreaddons_add_plugin(plasma_potd_simonstalenhagprovider SOURCES simonstalenhagprovider.cpp INSTALL_NAMESPACE "potd")
target_link_libraries( plasma_potd_simonstalenhagprovider plasmapotdprovidercore KF5::KIOCore )
