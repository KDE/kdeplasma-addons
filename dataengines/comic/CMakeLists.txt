project(plasma_comic_dataengine)
set(comic_engine_SRCS
    cachedprovider.cpp
    comic.cpp
)
find_package(KF5Kross)

add_library(plasma_engine_comic MODULE ${comic_engine_SRCS})

target_link_libraries(plasma_engine_comic plasmacomicprovidercore
    KF5::KDELibs4Support
    KF5::WidgetsAddons
    KF5::Solid
    KF5::Plasma
    KF5::KrossCore
    KF5::KrossUi
    KF5::I18n
)

kservice_desktop_to_json(plasma_engine_comic plasma-dataengine-comic.desktop)

install(TARGETS plasma_engine_comic DESTINATION ${PLUGIN_INSTALL_DIR}/plasma/dataengine)

install(FILES plasma-dataengine-comic.desktop DESTINATION ${SERVICES_INSTALL_DIR} )


set(comicpackage_SRCS
    comic_package.cpp
)

add_library(plasma_packagestructure_comic MODULE ${comicpackage_SRCS})
target_link_libraries( plasma_packagestructure_comic
    KF5::WidgetsAddons
    KF5::Plasma
    KF5::Service
    KF5::KrossCore
    KF5::KrossUi
    KF5::I18n
)
kservice_desktop_to_json(plasma_packagestructure_comic plasma-packagestructure-comic.desktop)
install(TARGETS plasma_packagestructure_comic DESTINATION ${PLUGIN_INSTALL_DIR}/plasma/packagestructure)

########### plugin core library ############

set(comic_provider_core_SRCS
  comicprovider.cpp
)

add_library(plasmacomicprovidercore SHARED ${comic_provider_core_SRCS})

target_link_libraries(plasmacomicprovidercore
    KF5::KDELibs4Support
    KF5::WidgetsAddons
    KF5::KIOCore
    KF5::KrossCore
    KF5::KrossUi
    KF5::I18n
)
#target_link_libraries( plasmacomicprovidercore LINK_INTERFACE_LIBRARIES
#    KF5::KDELibs4Support
#)
set_target_properties(plasmacomicprovidercore PROPERTIES VERSION 1.0.0 SOVERSION 1)

kservice_desktop_to_json(plasmacomicprovidercore plasma_comicprovider.desktop)

install( TARGETS plasmacomicprovidercore ${INSTALL_TARGETS_DEFAULT_ARGS} )

install( FILES plasma_comicprovider.desktop DESTINATION ${SERVICETYPES_INSTALL_DIR} )

########### kross ###############

set(plasma_comic_krossprovider_SRCS
  comicproviderkross.cpp
  comicproviderwrapper.cpp
  comic_package.cpp
)

add_library(plasma_comic_krossprovider MODULE ${plasma_comic_krossprovider_SRCS})
target_link_libraries(plasma_comic_krossprovider plasmacomicprovidercore
    KF5::KDELibs4Support
    Qt5::Gui
    KF5::KIOCore
    KF5::KrossCore
    KF5::KrossUi
    KF5::Plasma
    KF5::I18n
)

kservice_desktop_to_json(plasma_comic_krossprovider plasma-packagestructure-comic.desktop)

install( TARGETS plasma_comic_krossprovider DESTINATION ${PLUGIN_INSTALL_DIR} )
