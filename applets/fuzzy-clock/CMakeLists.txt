project(plasma-fuzzy-clock)

find_package(KDE4 REQUIRED)
include(KDE4Defaults)
find_package(Plasma REQUIRED)
 
add_definitions (${QT_DEFINITIONS} ${KDE4_DEFINITIONS})
include_directories(
   ${CMAKE_SOURCE_DIR}
   ${CMAKE_BINARY_DIR}
   ${KDE4_INCLUDES}
   )

set(fuzzyclock_SRCS fuzzyClock.cpp)

kde4_add_ui_files(fuzzyclock_SRCS fuzzyClockConfig.ui )

kde4_add_plugin(plasma_applet_fuzzy_clock ${fuzzyclock_SRCS})
target_link_libraries(plasma_applet_fuzzy_clock plasmaclock ${PLASMA_LIBS} ${KDE4_KDEUI_LIBS})

install(TARGETS plasma_applet_fuzzy_clock DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES plasma-clock-fuzzy.desktop DESTINATION ${SERVICES_INSTALL_DIR})
#install(FILES fuzzy-clock.svg DESTINATION ${DATA_INSTALL_DIR}/desktoptheme/default/widgets/)
