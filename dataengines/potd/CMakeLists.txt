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

install(TARGETS plasma_engine_potd DESTINATION ${PLUGIN_INSTALL_DIR}/plasma/dataengine )
install(FILES plasma-dataengine-potd.desktop DESTINATION ${SERVICES_INSTALL_DIR} )


########### plugin core library ############

set(potd_provider_core_SRCS
	potdprovider.cpp
	${CMAKE_CURRENT_BINARY_DIR}/plasma_potd_export.h
)

add_library( plasmapotdprovidercore SHARED ${potd_provider_core_SRCS} )
target_link_libraries( plasmapotdprovidercore Qt5::Gui KF5::CoreAddons )
set_target_properties( plasmapotdprovidercore PROPERTIES VERSION 1.0.0 SOVERSION 1 )
generate_export_header(plasmapotdprovidercore BASE_NAME PLASMA_POTD EXPORT_FILE_NAME plasma_potd_export.h)

install( TARGETS plasmapotdprovidercore ${INSTALL_TARGETS_DEFAULT_ARGS} )

########### plugins ###############

set( potd_flickr_provider_SRCS
	flickrprovider.cpp
)

add_library( plasma_potd_flickrprovider MODULE ${potd_flickr_provider_SRCS} )
target_link_libraries( plasma_potd_flickrprovider plasmapotdprovidercore KF5::KIOCore KF5::CoreAddons)

kcoreaddons_desktop_to_json(plasma_potd_flickrprovider flickrprovider.desktop SERVICE_TYPES ${CMAKE_CURRENT_SOURCE_DIR}/plasma_potdprovider.desktop)

install( TARGETS plasma_potd_flickrprovider DESTINATION ${PLUGIN_INSTALL_DIR}/potd )

set( potd_apod_provider_SRCS
	apodprovider.cpp
)

add_library( plasma_potd_apodprovider MODULE ${potd_apod_provider_SRCS} )
target_link_libraries( plasma_potd_apodprovider plasmapotdprovidercore KF5::KIOCore)

kcoreaddons_desktop_to_json(plasma_potd_apodprovider apodprovider.desktop SERVICE_TYPES ${CMAKE_CURRENT_SOURCE_DIR}/plasma_potdprovider.desktop)

install( TARGETS plasma_potd_apodprovider DESTINATION ${PLUGIN_INSTALL_DIR}/potd )

set( potd_natgeo_provider_SRCS
	natgeoprovider.cpp
)

add_library( plasma_potd_natgeoprovider MODULE ${potd_natgeo_provider_SRCS} )
target_link_libraries( plasma_potd_natgeoprovider plasmapotdprovidercore KF5::KIOCore)

kcoreaddons_desktop_to_json(plasma_potd_natgeoprovider natgeoprovider.desktop SERVICE_TYPES ${CMAKE_CURRENT_SOURCE_DIR}/plasma_potdprovider.desktop)

install( TARGETS plasma_potd_natgeoprovider DESTINATION ${PLUGIN_INSTALL_DIR}/potd )

set( potd_epod_provider_SRCS
	epodprovider.cpp
)

add_library( plasma_potd_epodprovider MODULE ${potd_epod_provider_SRCS} )
target_link_libraries( plasma_potd_epodprovider plasmapotdprovidercore KF5::KIOCore)

kcoreaddons_desktop_to_json(plasma_potd_epodprovider epodprovider.desktop SERVICE_TYPES ${CMAKE_CURRENT_SOURCE_DIR}/plasma_potdprovider.desktop)

install( TARGETS plasma_potd_epodprovider DESTINATION ${PLUGIN_INSTALL_DIR}/potd )

set( potd_noaa_provider_SRCS
	noaaprovider.cpp
)

add_library( plasma_potd_noaaprovider MODULE ${potd_noaa_provider_SRCS} )
target_link_libraries( plasma_potd_noaaprovider plasmapotdprovidercore KF5::KIOCore)

kcoreaddons_desktop_to_json(plasma_potd_noaaprovider noaaprovider.desktop SERVICE_TYPES ${CMAKE_CURRENT_SOURCE_DIR}/plasma_potdprovider.desktop)

install( TARGETS plasma_potd_noaaprovider DESTINATION ${PLUGIN_INSTALL_DIR}/potd )

set( potd_wcpotd_provider_SRCS
	wcpotdprovider.cpp
)

add_library( plasma_potd_wcpotdprovider MODULE ${potd_wcpotd_provider_SRCS} )
target_link_libraries( plasma_potd_wcpotdprovider plasmapotdprovidercore KF5::KIOCore )

kcoreaddons_desktop_to_json(plasma_potd_wcpotdprovider wcpotdprovider.desktop SERVICE_TYPES ${CMAKE_CURRENT_SOURCE_DIR}/plasma_potdprovider.desktop)

install( TARGETS plasma_potd_wcpotdprovider DESTINATION ${PLUGIN_INSTALL_DIR}/potd )
