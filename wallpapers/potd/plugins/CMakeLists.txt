set(potd_engine_SRCS
	cachedprovider.cpp
	potdbackend.cpp
	potdengine.cpp
	potdprovidermodel.cpp
	potdplugin.cpp
)

ecm_qt_declare_logging_category(potd_engine_SRCS
                                HEADER debug.h
                                IDENTIFIER WALLPAPERPOTD
                                CATEGORY_NAME kde.wallpapers.potd
                                DEFAULT_SEVERITY Info)

add_library(plasma_wallpaper_potdplugin SHARED ${potd_engine_SRCS})
target_link_libraries(plasma_wallpaper_potdplugin plasmapotdprovidercore
    KF5::I18n
    KF5::KIOCore
    Qt::DBus
    Qt::Qml
    Qt::Widgets # QFileDialog
)
install(TARGETS plasma_wallpaper_potdplugin DESTINATION ${KDE_INSTALL_QMLDIR}/org/kde/plasma/wallpapers/potd)
install(FILES qmldir DESTINATION ${KDE_INSTALL_QMLDIR}/org/kde/plasma/wallpapers/potd)

########### plugin core library ############
set(POTDPROVIDER_VERSION 1.0.0)
set(POTDPROVIDER_VERSION_MAJOR 1)

set(potd_provider_core_SRCS
	potdprovider.cpp
	${CMAKE_CURRENT_BINARY_DIR}/plasma_potd_export.h
)

add_library(plasmapotdprovidercore SHARED ${potd_provider_core_SRCS})
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

install(TARGETS plasmapotdprovidercore EXPORT plasmapotdproviderTargets ${KDE_INSTALL_TARGETS_DEFAULT_ARGS})
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

add_subdirectory(providers)
