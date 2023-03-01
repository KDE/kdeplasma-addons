plasma_install_package(package org.kde.plasma.mediaframe)

set(mediaframeplugin_SRCS
    plugin/mediaframe.cpp
    plugin/mediaframeplugin.cpp
)

add_library(mediaframeplugin SHARED ${mediaframeplugin_SRCS})

target_link_libraries(mediaframeplugin
                      Qt::Core
                      Qt::Qml
                      Qt::Quick
                      KF6::I18n
                      KF6::KIOCore
)

install(TARGETS mediaframeplugin DESTINATION ${KDE_INSTALL_QMLDIR}/org/kde/plasma/private/mediaframe)
install(FILES plugin/qmldir DESTINATION ${KDE_INSTALL_QMLDIR}/org/kde/plasma/private/mediaframe)





