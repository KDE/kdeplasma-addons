# Project Needs a name ofcourse
project(plasma-knowledgebase)
 
 
# We add our source code here
set(knowledgebase_SRCS kbitemwidget.cpp knowledgebase.cpp kbitemtitle.cpp)

kde4_add_ui_files(knowledgebase_SRCS knowledgeBaseConfig.ui )

# Now make sure all files get to the right place
kde4_add_plugin(plasma_applet_knowledgebase ${knowledgebase_SRCS})
target_link_libraries(plasma_applet_knowledgebase 
                      ${KDE4_PLASMA_LIBS} ${KDE4_KDEUI_LIBS} ${KDE4_KIO_LIBS})
 
install(TARGETS plasma_applet_knowledgebase
        DESTINATION ${PLUGIN_INSTALL_DIR})
 
install(FILES plasma-applet-knowledgebase.desktop
        DESTINATION ${SERVICES_INSTALL_DIR})
