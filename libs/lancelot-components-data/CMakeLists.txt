project(lancelotcomponentsdata)

include(KDE4Defaults)

set(lancelotcomponentsdata_SRCS
    lancelotcomponentsdataplugin.cpp
    BaseModel.cpp
    FavoriteApplications.cpp
    FolderModel.cpp
    DirModel.cpp
    SystemServices.cpp
    NewDocuments.cpp
    RecentDocuments.cpp
    Devices.cpp
    ContactsKopete.cpp
    )

INCLUDE_DIRECTORIES(
    ${CMAKE_SOURCE_DIR}
    ${CMAKE_BINARY_DIR}
    ${KDE4_INCLUDES}
    )

qt4_automoc(${lancelotcomponentsdata_SRCS})

#################################################

# Kopete

set(kopete_xml ${DBUS_INTERFACES_INSTALL_DIR}/org.kde.Kopete.xml)
if (NOT EXISTS ${kopete_xml})
   set(kopete_xml external/org.kde.Kopete.xml)
endif (NOT EXISTS ${kopete_xml})
qt4_add_dbus_interface(lancelotcomponentsdata_SRCS ${kopete_xml} kopete_interface)


#################################################

add_library(
    lancelotcomponentsdataplugin
    SHARED
    ${lancelotcomponentsdata_SRCS}
    )

target_link_libraries(
    lancelotcomponentsdataplugin
    ${QT_QTCORE_LIBRARY}
    ${QT_QTGUI_LIBRARY}
    ${QT_QTDECLARATIVE_LIBRARY}
    ${KDE4_KDEUI_LIBS}
    ${KDE4_KFILE_LIBS}
    ${KDE4_KIO_LIBS}
    ${KDE4_PLASMA_LIBS}
    ${KDE4_SOLID_LIBS}
    )

install(
    TARGETS
    lancelotcomponentsdataplugin
    DESTINATION
    ${IMPORTS_INSTALL_DIR}/org/kde/lancelot/components/data
    )

install(
    FILES
    qmldir
    DESTINATION
    ${IMPORTS_INSTALL_DIR}/org/kde/lancelot/components/data
    )
