set(comic_engine_SRCS
    cachedprovider.cpp
    comic.cpp
    comicproviderkross.cpp
    comicproviderwrapper.cpp
)

kcoreaddons_add_plugin(plasma_engine_comic SOURCES ${comic_engine_SRCS} INSTALL_NAMESPACE "plasma/dataengine")

target_link_libraries(plasma_engine_comic plasmacomicprovidercore
    KF5::WidgetsAddons
    KF5::Plasma
    KF5::KrossCore
    KF5::KrossUi
    KF5::I18n
)

set(comicpackage_SRCS
    comic_package.cpp
)

kcoreaddons_add_plugin(plasma_packagestructure_comic SOURCES ${comicpackage_SRCS} INSTALL_NAMESPACE "kpackage/packagestructure")
target_link_libraries( plasma_packagestructure_comic
    KF5::WidgetsAddons
    KF5::Plasma
    KF5::KrossCore
    KF5::KrossUi
    KF5::I18n
)

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

install( TARGETS plasmacomicprovidercore ${KDE_INSTALL_TARGETS_DEFAULT_ARGS} LIBRARY NAMELINK_SKIP )

########### kross ###############

set(plasma_comic_krossprovider_SRCS
  comicproviderkross.cpp
  comicproviderwrapper.cpp
  comic_package.cpp
)

add_library(plasma_comic_krossprovider MODULE ${plasma_comic_krossprovider_SRCS})
target_link_libraries(plasma_comic_krossprovider plasmacomicprovidercore
    Qt::Gui
    KF5::KIOCore
    KF5::KrossCore
    KF5::KrossUi
    KF5::Plasma
    KF5::I18n
)

install( TARGETS plasma_comic_krossprovider DESTINATION ${KDE_INSTALL_PLUGINDIR}/plasma/dataengine)
