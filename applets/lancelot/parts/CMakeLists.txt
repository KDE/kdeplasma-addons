project(LancelotMenu)

include_directories(
   ${CMAKE_SOURCE_DIR}
   ${CMAKE_BINARY_DIR}
   ${CMAKE_CURRENT_SOURCE_DIR}
   ${CMAKE_CURRENT_BINARY_DIR}
   ${KDE4_INCLUDES}

   ${LANCELOT_INCLUDE_DIR}
   ${LANCELOT_DATAMODELS_INCLUDE_DIR}
)

# Lancelot Part Applet

set(plasma_applet_lancelot_part_SRCS
   ${plasma_lancelot_models_SRCS}
   LancelotPart.cpp
   PartsMergedModel.cpp
   LancelotPartConfig.cpp
)

kde4_add_ui_files(
   plasma_applet_lancelot_part_SRCS
   LancelotPartConfigBase.ui
)

kde4_add_plugin(plasma_applet_lancelot_part ${plasma_applet_lancelot_part_SRCS})

target_link_libraries(
   plasma_applet_lancelot_part
   ${KDE4_PLASMA_LIBS}
   ${KDE4_KIO_LIBS}
   ${LANCELOT_LIBS}
   ${LANCELOT_DATAMODELS_LIBS}
   )

install(
   TARGETS plasma_applet_lancelot_part
   DESTINATION ${PLUGIN_INSTALL_DIR}
)

install(
   FILES plasma-applet-lancelot-part.desktop
   DESTINATION ${SERVICES_INSTALL_DIR}
)

install(
   FILES lancelotpart-mime.xml
   DESTINATION ${XDG_MIME_INSTALL_DIR}
)

update_xdg_mimetypes(${XDG_MIME_INSTALL_DIR})

