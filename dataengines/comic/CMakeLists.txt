set(comic_engine_SRCS
    cachedprovider.cpp
    comic.cpp
    comicproviderkross.cpp
    comicproviderwrapper.cpp
)

add_library(plasma_engine_comic MODULE ${comic_engine_SRCS})

target_link_libraries(plasma_engine_comic plasmacomicprovidercore
    KF5::WidgetsAddons
    KF5::Plasma
    KF5::KrossCore
    KF5::KrossUi
    KF5::I18n
)

kcoreaddons_desktop_to_json(plasma_engine_comic plasma-dataengine-comic.desktop SERVICE_TYPES plasma-dataengine.desktop)

install(TARGETS plasma_engine_comic DESTINATION ${KDE_INSTALL_PLUGINDIR}/plasma/dataengine)

install(FILES plasma-dataengine-comic.desktop DESTINATION ${KDE_INSTALL_KSERVICES5DIR} )


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
kcoreaddons_desktop_to_json(plasma_packagestructure_comic plasma-packagestructure-comic.desktop SERVICE_TYPES plasma-packagestructure.desktop)
install(TARGETS plasma_packagestructure_comic DESTINATION ${KDE_INSTALL_PLUGINDIR}/kpackage/packagestructure)

########### plugin core library ############

set(comic_provider_core_SRCS
  comicprovider.cpp
)

add_library(plasmacomicprovidercore SHARED ${comic_provider_core_SRCS})
generate_export_header(plasmacomicprovidercore EXPORT_FILE_NAME plasma_comic_export.h EXPORT_MACRO_NAME PLASMA_COMIC_EXPORT)

target_link_libraries(plasmacomicprovidercore
    KF5::WidgetsAddons
    KF5::KIOCore
    KF5::KrossCore
    KF5::KrossUi
    KF5::I18n
)
set_target_properties(plasmacomicprovidercore PROPERTIES VERSION 1.0.0 SOVERSION 1)

install( TARGETS plasmacomicprovidercore ${INSTALL_TARGETS_DEFAULT_ARGS} LIBRARY NAMELINK_SKIP )

install( FILES plasma_comicprovider.desktop DESTINATION ${KDE_INSTALL_KSERVICETYPES5DIR} )

########### kross ###############

set(plasma_comic_krossprovider_SRCS
  comicproviderkross.cpp
  comicproviderwrapper.cpp
  comic_package.cpp
)

add_library(plasma_comic_krossprovider MODULE ${plasma_comic_krossprovider_SRCS})
target_link_libraries(plasma_comic_krossprovider plasmacomicprovidercore
    Qt5::Gui
    KF5::KIOCore
    KF5::KrossCore
    KF5::KrossUi
    KF5::Plasma
    KF5::I18n
)

kcoreaddons_desktop_to_json(plasma_comic_krossprovider plasma-packagestructure-comic.desktop SERVICE_TYPES plasma-packagestructure.desktop)

install( TARGETS plasma_comic_krossprovider DESTINATION ${KDE_INSTALL_PLUGINDIR} )
