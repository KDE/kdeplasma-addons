project(LancelotMenu)

#########################################################
# Checking for Composite Begin                          #
#########################################################

if(X11_Xrender_FOUND )
   set( LANCELOT_HAVE_COMPOSITING 1 )
endif( X11_Xrender_FOUND )

macro_log_feature(LANCELOT_HAVE_COMPOSITING
   "XRender"
   "Lancelot compositing support"
   "http://www.x.org/"
   FALSE
   ""
   "STRONGLY_RECOMMENDED: XRender extension is required for Lancelot compositing support"
)

configure_file(config-lancelot.h.cmake
   ${CMAKE_CURRENT_BINARY_DIR}/config-lancelot.h )

#########################################################
# Checking for Composite End                            #
#########################################################

# include(PlasmaUIMacros)

include_directories(
   ${CMAKE_SOURCE_DIR}
   ${CMAKE_BINARY_DIR}
   ${CMAKE_CURRENT_SOURCE_DIR}
   ${CMAKE_CURRENT_BINARY_DIR}
   ${KDE4_INCLUDES}

   ${LANCELOT_INCLUDE_DIR}
   ${LANCELOT_DATAMODELS_INCLUDE_DIR}
)

# Lancelot application files
set(plasma_lancelot_application_SRCS
   LancelotApplication.cpp
   LancelotWindow.cpp
   LancelotConfig.cpp
   QtDisplay.cpp
   AboutData.cpp
)

# This is done manually now:
# plasmaui_add_ui_files(plasma_lancelot_application_SRCS
#    "ui/LancelotWindowBase.xml"
# )

set(ksmserver_xml ${KDE4WORKSPACE_DBUS_INTERFACES_DIR}/org.kde.KSMServerInterface.xml)
qt4_add_dbus_interface(plasma_lancelot_application_SRCS ${ksmserver_xml} ksmserver_interface)

qt4_add_dbus_adaptor(plasma_lancelot_application_SRCS org.kde.lancelot.xml
   LancelotApplication.h LancelotApplication)

# Lancelot Menu
set(
   plasma_lancelot_SRCS
   main.cpp

   ${plasma_lancelot_application_SRCS}
)

kde4_add_ui_files(
   plasma_lancelot_SRCS
   ui/LancelotConfigBase.ui
)

kde4_add_executable(lancelot-menu ${plasma_lancelot_SRCS})

target_link_libraries(
   lancelot-menu
   ${KDE4_KIO_LIBS}
   ${KDE4_PLASMA_LIBS}
   ${KDE4_KCMUTILS_LIBS}
   ${LANCELOT_LIBS}
   ${LANCELOT_DATAMODELS_LIBS}
   )

if(LANCELOT_HAVE_COMPOSITING)
   target_link_libraries(lancelot-menu ${X11_Xrender_LIB} ${X11_X11_LIB})
endif(LANCELOT_HAVE_COMPOSITING)

set_target_properties(lancelot-menu PROPERTIES OUTPUT_NAME lancelot)

########### install applicaiton ###############

# To regenerate .xml file:
# qdbuscpp2xml -M -s LancelotApplication.h -o org.kde.lancelot.xml.new

install(TARGETS lancelot-menu ${INSTALL_TARGETS_DEFAULT_ARGS} )

install(FILES lancelot.notifyrc DESTINATION ${DATA_INSTALL_DIR}/lancelot)

install(FILES lancelot.desktop DESTINATION ${SERVICES_INSTALL_DIR})

