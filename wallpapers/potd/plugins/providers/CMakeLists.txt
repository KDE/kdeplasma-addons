include_directories(${CMAKE_CURRENT_BINARY_DIR}/.. ${CMAKE_CURRENT_SOURCE_DIR}/..)

kcoreaddons_add_plugin(plasma_potd_flickrprovider SOURCES flickrprovider.cpp INSTALL_NAMESPACE "potd")
target_link_libraries(plasma_potd_flickrprovider plasmapotdprovidercore KF5::KIOCore KF5::CoreAddons)

kcoreaddons_add_plugin(plasma_potd_apodprovider SOURCES apodprovider.cpp INSTALL_NAMESPACE "potd")
target_link_libraries(plasma_potd_apodprovider plasmapotdprovidercore KF5::KIOCore)

kcoreaddons_add_plugin(plasma_potd_natgeoprovider SOURCES natgeoprovider.cpp INSTALL_NAMESPACE "potd")
target_link_libraries(plasma_potd_natgeoprovider plasmapotdprovidercore KF5::KIOCore)

kcoreaddons_add_plugin(plasma_potd_epodprovider SOURCES epodprovider.cpp INSTALL_NAMESPACE "potd")
target_link_libraries(plasma_potd_epodprovider plasmapotdprovidercore KF5::KIOCore)

kcoreaddons_add_plugin(plasma_potd_noaaprovider SOURCES noaaprovider.cpp INSTALL_NAMESPACE "potd")
target_link_libraries(plasma_potd_noaaprovider plasmapotdprovidercore KF5::KIOCore)

kcoreaddons_add_plugin(plasma_potd_wcpotdprovider SOURCES wcpotdprovider.cpp INSTALL_NAMESPACE "potd")
target_link_libraries(plasma_potd_wcpotdprovider plasmapotdprovidercore KF5::KIOCore)

kcoreaddons_add_plugin(plasma_potd_bingprovider SOURCES bingprovider.cpp INSTALL_NAMESPACE "potd")
target_link_libraries(plasma_potd_bingprovider plasmapotdprovidercore KF5::KIOCore)

kcoreaddons_add_plugin(plasma_potd_unsplashprovider SOURCES unsplashprovider.cpp INSTALL_NAMESPACE "potd")
target_link_libraries(plasma_potd_unsplashprovider plasmapotdprovidercore KF5::KIOCore)
