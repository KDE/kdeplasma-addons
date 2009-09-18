# Project Needs a name ofcourse
project(plasma-magnifique)
 
# We add our source code here
set(magnifique_SRCS magnifique.cpp)
 
# Now make sure all files get to the right place
kde4_add_plugin(plasma_applet_magnifique ${magnifique_SRCS})
target_link_libraries(plasma_applet_magnifique 
                      ${KDE4_PLASMA_LIBS} ${KDE4_KDEUI_LIBS})
 
install(TARGETS plasma_applet_magnifique
        DESTINATION ${PLUGIN_INSTALL_DIR})
 
install(FILES plasma-applet-magnifique.desktop
        DESTINATION ${SERVICES_INSTALL_DIR})
