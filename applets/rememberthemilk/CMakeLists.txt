project(rememberthemilk-plasmoid)

include_directories(
   ${CMAKE_CURRENT_SOURCE_DIR}
   ${CMAKE_CURRENT_BINARY_DIR}
   ${KDE4_INCLUDES}
   )

set(rememberthemilk-plasmoid_SRCS 
      rememberthemilk-plasmoid.cpp
      taskitem.cpp
      taskitemdelegate.cpp
      taskeditor.cpp
      taskmodel.cpp
      tasksortfilter.cpp)
      
kde4_add_ui_files(rememberthemilk-plasmoid_SRCS authenticate.ui general.ui)

kde4_add_plugin(plasma_applet_rtm ${rememberthemilk-plasmoid_SRCS})
target_link_libraries(plasma_applet_rtm 
                      ${KDE4_PLASMA_LIBS}
                      ${KDE4_SOLID_LIBS}
                      ${KDE4_KDEUI_LIBS}
                      ${KDE4_KDEWEBKIT_LIBS}
                     )
 
install(TARGETS plasma_applet_rtm
        DESTINATION ${PLUGIN_INSTALL_DIR})
 
install(FILES plasma-applet-rememberthemilk.desktop
        DESTINATION ${SERVICES_INSTALL_DIR})
