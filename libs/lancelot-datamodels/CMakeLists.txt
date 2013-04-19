project(liblancelot-datamodels)

find_package(KDE4 REQUIRED)

include (KDE4Defaults)
add_definitions (${QT_DEFINITIONS} ${KDE4_DEFINITIONS})

find_package(Lancelot REQUIRED)

set (ADDITIONAL_LINK_LIBRARIES)

########### are akonadi and pimlibs available? ##########

# find_package(KdepimLibs)

if (KDEPIMLIBS_FOUND)
   set( LANCELOT_DATAMODELS_HAS_PIMLIBS 1 )

   include_directories(
      ${AKONADI_INCLUDE_DIR}
      ${KDEPIMLIBS_INCLUDE_DIRS}
   )

   set (ADDITIONAL_LINK_LIBRARIES
        ${ADDITIONAL_LINK_LIBRARIES}
        ${KDEPIMLIBS_AKONADI_LIBS}
   )
endif(KDEPIMLIBS_FOUND)

macro_log_feature(LANCELOT_DATAMODELS_HAS_PIMLIBS
   "Akonadi and KDE PIM Libs"
   "Support for unread messages model"
   "http://www.kde.org/"
   FALSE
   ""
   "STRONGLY_RECOMMENDED: Akonadi and KDE PIM libraries are required for messages datamodel and kmail support"
)

configure_file(config-lancelot-datamodels.h.cmake
   ${CMAKE_CURRENT_BINARY_DIR}/config-lancelot-datamodels.h )



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
   ${Boost_INCLUDE_DIR}
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
   AvailableModels.cpp
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

set(screensaver_xml ${KDE4WORKSPACE_DBUS_INTERFACES_DIR}/org.freedesktop.ScreenSaver.xml)
if (NOT EXISTS ${screensaver_xml})
   set(screensaver_xml external/org.freedesktop.ScreenSaver.xml)
endif (NOT EXISTS ${screensaver_xml})
qt4_add_dbus_interface(lancelot_datamodels_LIB_SRCS ${screensaver_xml} screensaver_interface)

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
   ${ADDITIONAL_LINK_LIBRARIES}
   kworkspace
)

set_target_properties(lancelot-datamodels PROPERTIES VERSION 1.7.0 SOVERSION 1)
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
   AvailableModels.h
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
   Lancelot/Models/Applications
   Lancelot/Models/AvailableModels
   Lancelot/Models/BaseMergedModel
   Lancelot/Models/BaseModel
   Lancelot/Models/ContactsKopete
   Lancelot/Models/Devices
   Lancelot/Models/FavoriteApplications
   Lancelot/Models/FolderModel
   Lancelot/Models/Logger
   Lancelot/Models/MessagesKmail
   Lancelot/Models/NewDocuments
   Lancelot/Models/OpenDocuments
   Lancelot/Models/Places
   Lancelot/Models/RecentDocuments
   Lancelot/Models/Runner
   Lancelot/Models/Serializator
   Lancelot/Models/SystemActions
   Lancelot/Models/SystemServices
   Lancelot/Models/XbelModel
   DESTINATION ${INCLUDE_INSTALL_DIR}/KDE/Lancelot/Models COMPONENT Devel
)

