project(plasma-fifteenPuzzle)

add_subdirectory(icons)
include_directories( ${KDE4_INCLUDES} )

set(fifteenPuzzle_SRCS src/fifteenPuzzleConfig.cpp src/fifteenPuzzle.cpp src/fifteen.cpp src/piece.cpp )

kde4_add_ui_files(fifteenPuzzle_SRCS src/fifteenPuzzleConfig.ui )
kde4_add_plugin(plasma_applet_fifteenPuzzle ${fifteenPuzzle_SRCS})

target_link_libraries(plasma_applet_fifteenPuzzle ${KDE4_PLASMA_LIBS} ${KDE4_KIO_LIBS})


install(TARGETS plasma_applet_fifteenPuzzle DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES plasma-applet-fifteenPuzzle.desktop DESTINATION ${SERVICES_INSTALL_DIR})
install(FILES images/greensquare.svgz DESTINATION ${DATA_INSTALL_DIR}/desktoptheme/default/fifteenPuzzle/)

