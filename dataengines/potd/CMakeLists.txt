set(potd_engine_SRCS
	cachedprovider.cpp
	potd.cpp
)

add_library(plasma_engine_potd MODULE ${potd_engine_SRCS} )
target_link_libraries(plasma_engine_potd plasmapotdprovidercore
    KF5::Plasma
    KF5::KIOCore
)

kcoreaddons_desktop_to_json(plasma_engine_potd plasma-dataengine-potd.desktop SERVICE_TYPES plasma-dataengine.desktop)

install(TARGETS plasma_engine_potd DESTINATION ${KDE_INSTALL_PLUGINDIR}/plasma/dataengine )
install(FILES plasma-dataengine-potd.desktop DESTINATION ${KDE_INSTALL_KSERVICES5DIR} )


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
target_link_libraries( plasmapotdprovidercore Qt5::Gui KF5::CoreAddons )
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

set( potd_flickr_provider_SRCS
	flickrprovider.cpp
)

add_library( plasma_potd_flickrprovider MODULE ${potd_flickr_provider_SRCS} )
target_link_libraries( plasma_potd_flickrprovider plasmapotdprovidercore KF5::KIOCore KF5::CoreAddons)

install( TARGETS plasma_potd_flickrprovider DESTINATION ${KDE_INSTALL_PLUGINDIR}/potd )

set( potd_apod_provider_SRCS
	apodprovider.cpp
)

add_library( plasma_potd_apodprovider MODULE ${potd_apod_provider_SRCS} )
target_link_libraries( plasma_potd_apodprovider plasmapotdprovidercore KF5::KIOCore)

install( TARGETS plasma_potd_apodprovider DESTINATION ${KDE_INSTALL_PLUGINDIR}/potd )

set( potd_natgeo_provider_SRCS
	natgeoprovider.cpp
)

add_library( plasma_potd_natgeoprovider MODULE ${potd_natgeo_provider_SRCS} )
target_link_libraries( plasma_potd_natgeoprovider plasmapotdprovidercore KF5::KIOCore)

install( TARGETS plasma_potd_natgeoprovider DESTINATION ${KDE_INSTALL_PLUGINDIR}/potd )

set( potd_epod_provider_SRCS
	epodprovider.cpp
)

add_library( plasma_potd_epodprovider MODULE ${potd_epod_provider_SRCS} )
target_link_libraries( plasma_potd_epodprovider plasmapotdprovidercore KF5::KIOCore)

install( TARGETS plasma_potd_epodprovider DESTINATION ${KDE_INSTALL_PLUGINDIR}/potd )

set( potd_noaa_provider_SRCS
	noaaprovider.cpp
)

add_library( plasma_potd_noaaprovider MODULE ${potd_noaa_provider_SRCS} )
target_link_libraries( plasma_potd_noaaprovider plasmapotdprovidercore KF5::KIOCore)

install( TARGETS plasma_potd_noaaprovider DESTINATION ${KDE_INSTALL_PLUGINDIR}/potd )

set( potd_wcpotd_provider_SRCS
	wcpotdprovider.cpp
)

add_library( plasma_potd_wcpotdprovider MODULE ${potd_wcpotd_provider_SRCS} )
target_link_libraries( plasma_potd_wcpotdprovider plasmapotdprovidercore KF5::KIOCore )

install( TARGETS plasma_potd_wcpotdprovider DESTINATION ${KDE_INSTALL_PLUGINDIR}/potd )

set( potd_bing_provider_SRCS
	bingprovider.cpp
)

add_library( plasma_potd_bingprovider MODULE ${potd_bing_provider_SRCS} )
target_link_libraries( plasma_potd_bingprovider plasmapotdprovidercore KF5::KIOCore )

install( TARGETS plasma_potd_bingprovider DESTINATION ${KDE_INSTALL_PLUGINDIR}/potd )

set( potd_unsplash_provider_SRCS
	unsplashprovider.cpp
)

add_library( plasma_potd_unsplashprovider MODULE ${potd_unsplash_provider_SRCS} )
target_link_libraries( plasma_potd_unsplashprovider plasmapotdprovidercore KF5::KIOCore )

install( TARGETS plasma_potd_unsplashprovider DESTINATION ${KDE_INSTALL_PLUGINDIR}/potd )
