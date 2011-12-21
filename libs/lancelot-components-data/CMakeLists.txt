project(lancelotcomponentsdata)

include(KDE4Defaults)

set(lancelotcomponentsdata_SRCS
    lancelotcomponentsdataplugin.cpp
    BaseModel.cpp

    ContactsKopete.cpp
    Devices.cpp
    DirModel.cpp
    FavoriteApplications.cpp
    FolderModel.cpp
    MessagesAkonadi.cpp
    NewDocuments.cpp
    OpenDocuments.cpp
    Places.cpp
    RecentDocuments.cpp
    Runner.cpp
    # SystemActions.cpp
    SystemServices.cpp
    XbelModel.cpp
    )

INCLUDE_DIRECTORIES(
    ${CMAKE_SOURCE_DIR}
    ${CMAKE_BINARY_DIR}
    ${KDE4_INCLUDES}
    )

qt4_automoc(${lancelotcomponentsdata_SRCS})

#################################################

set (ADDITIONAL_LINK_LIBRARIES)

# Kopete

set(kopete_xml ${DBUS_INTERFACES_INSTALL_DIR}/org.kde.Kopete.xml)
if (NOT EXISTS ${kopete_xml})
   set(kopete_xml external/org.kde.Kopete.xml)
endif (NOT EXISTS ${kopete_xml})
qt4_add_dbus_interface(lancelotcomponentsdata_SRCS ${kopete_xml} kopete_interface)

# KDE PIM

# find_package(KdepimLibs)

if (KDEPIMLIBS_FOUND)
   set( LANCELOT_DATA_HAS_PIMLIBS 1 )

   include_directories(
      ${AKONADI_INCLUDE_DIR}
      ${KDEPIMLIBS_INCLUDE_DIRS}
   )

   set (ADDITIONAL_LINK_LIBRARIES
        ${ADDITIONAL_LINK_LIBRARIES}
        ${KDEPIMLIBS_AKONADI_LIBS}
   )
endif(KDEPIMLIBS_FOUND)

macro_log_feature(LANCELOT_DATA_HAS_PIMLIBS
   "Akonadi and KDE PIM Libs"
   "Support for unread messages model"
   "http://www.kde.org/"
   FALSE
   ""
   "STRONGLY_RECOMMENDED: Akonadi and KDE PIM libraries are required for messages datamodel and kmail support"
)


#################################################

configure_file(config-lancelot-components-data.h.cmake
   ${CMAKE_CURRENT_BINARY_DIR}/config-lancelot-components-data.h )

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
    ${ADDITIONAL_LINK_LIBRARIES}
    ${KDE4WORKSPACE_TASKMANAGER_LIBRARY}
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
