project(liblancelot-datamodels)

find_package(KDE4 REQUIRED)

include (KDE4Defaults)
add_definitions (${QT_DEFINITIONS} ${KDE4_DEFINITIONS})

find_package(Lancelot REQUIRED)

########### define variables ##########

set(LANCELOT_DATAMODELS_INCLUDE_DIR
   "${CMAKE_CURRENT_SOURCE_DIR}/.."
   "${CMAKE_CURRENT_SOURCE_DIR}/"
   CACHE STRING
   "Location of liblancelot-datamodels headers"
   FORCE)

set(LANCELOT_DATAMODELS_LIBS
   "lancelot-datamodels"
   CACHE STRING
   "Location of liblancelot-datamodels binary"
   FORCE)

include_directories(
   ${CMAKE_SOURCE_DIR}
   ${CMAKE_BINARY_DIR}
   ${CMAKE_CURRENT_SOURCE_DIR}
   ${CMAKE_CURRENT_BINARY_DIR}
   ${KDE4_INCLUDES}

   ${LANCELOT_DATAMODELS_INCLUDE_DIR}
)

add_definitions(-DKDE_DEFAULT_DEBUG_AREA=1209)

########### libLancelot-datamodels ###############

set(lancelot_datamodels_LIB_SRCS
   Applications.cpp
   BaseMergedModel.cpp
   BaseModel.cpp
   ContactsKopete.cpp
   Devices.cpp
   FavoriteApplications.cpp
   FolderModel.cpp
   Logger.cpp
   MessagesKmail.cpp
   NewDocuments.cpp
   OpenDocuments.cpp
   Places.cpp
   RecentDocuments.cpp
   Runner.cpp
   Serializator.cpp
   SystemActions.cpp
   SystemServices.cpp
   XbelModel.cpp
)

set(screensaver_xml ${DBUS_INTERFACES_INSTALL_DIR}/org.freedesktop.ScreenSaver.xml)
qt4_add_dbus_interface(lancelot_datamodels_LIB_SRCS ${screensaver_xml} screensaver_interface)

set(kmail_xml ${DBUS_INTERFACES_INSTALL_DIR}/org.kde.kmail.kmail.xml)
if (NOT EXISTS ${kmail_xml})
   set(kmail_xml external/org.kde.kmail.kmail.xml)
endif (NOT EXISTS ${kmail_xml})
qt4_add_dbus_interface(lancelot_datamodels_LIB_SRCS ${kmail_xml} kmail_interface)

set(kmail_folder_xml ${DBUS_INTERFACES_INSTALL_DIR}/org.kde.kmail.folder.xml)
if (NOT EXISTS ${kmail_folder_xml})
   set(kmail_folder_xml external/org.kde.kmail.folder.xml)
endif (NOT EXISTS ${kmail_folder_xml})
qt4_add_dbus_interface(lancelot_datamodels_LIB_SRCS ${kmail_folder_xml} kmailfolder_interface)

set(kopete_xml ${DBUS_INTERFACES_INSTALL_DIR}/org.kde.Kopete.xml)
if (NOT EXISTS ${kopete_xml})
   set(kopete_xml external/org.kde.Kopete.xml)
endif (NOT EXISTS ${kopete_xml})
qt4_add_dbus_interface(lancelot_datamodels_LIB_SRCS ${kopete_xml} kopete_interface)


kde4_add_library(lancelot-datamodels SHARED ${lancelot_datamodels_LIB_SRCS})

target_link_libraries(
   lancelot-datamodels
   ${KDE4_KFILE_LIBS}
   ${KDE4_KIO_LIBS}
   ${KDE4_PLASMA_LIBS}
   ${KDE4_SOLID_LIBS}
   ${KDE4WORKSPACE_TASKMANAGER_LIBRARY}
   ${LANCELOT_LIBS}
   solidcontrol
   kworkspace
)

set_target_properties(lancelot-datamodels PROPERTIES VERSION 1.7.0 SOVERSION 0)
install(TARGETS lancelot-datamodels ${INSTALL_TARGETS_DEFAULT_ARGS})

########### install files ###############

set(lancelot_datamodels_LIB_INCLUDES
   Applications.h
   BaseMergedModel.h
   BaseModel.h
   ContactsKopete.h
   Devices.h
   FavoriteApplications.h
   FolderModel.h
   Logger.h
   MessagesKmail.h
   NewDocuments.h
   OpenDocuments.h
   Places.h
   RecentDocuments.h
   Runner.h
   Serializator.h
   SystemActions.h
   SystemServices.h
   XbelModel.h
)

install(FILES
   ${lancelot_datamodels_LIB_INCLUDES}
   DESTINATION ${INCLUDE_INSTALL_DIR}/lancelot-datamodels COMPONENT Devel
)

install(FILES
   Lancelot/DataModels/Applications
   Lancelot/DataModels/BaseMergedModel
   Lancelot/DataModels/BaseModel
   Lancelot/DataModels/ContactsKopete
   Lancelot/DataModels/Devices
   Lancelot/DataModels/FavoriteApplications
   Lancelot/DataModels/FolderModel
   Lancelot/DataModels/Logger
   Lancelot/DataModels/MessagesKmail
   Lancelot/DataModels/NewDocuments
   Lancelot/DataModels/OpenDocuments
   Lancelot/DataModels/Places
   Lancelot/DataModels/RecentDocuments
   Lancelot/DataModels/Runner
   Lancelot/DataModels/Serializator
   Lancelot/DataModels/SystemActions
   Lancelot/DataModels/SystemServices
   Lancelot/DataModels/XbelModel
   DESTINATION ${INCLUDE_INSTALL_DIR}/KDE/Lancelot/DataModels COMPONENT Devel
)

