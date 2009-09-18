project(plasma-paste)

set(paste_SRCS
    paste.cpp
    sendkeys.cpp
    list.cpp
    configdata.cpp
    snippetconfig.cpp
    autopasteconfig.cpp
    appkey.cpp
    pastemacroexpander.cpp
    addmacro.cpp
)

kde4_add_ui_files(paste_SRCS
    list.ui
    snippetconfig.ui
    autopasteconfig.ui
    appkey.ui
)

kde4_add_plugin(plasma_applet_paste ${paste_SRCS})
target_link_libraries(plasma_applet_paste
        ${X11_X11_LIB} ${KDE4_KDEUI_LIBS} ${KDE4_KIO_LIBS} ${KDE4_PLASMA_LIBS})

install(TARGETS plasma_applet_paste DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES plasma-applet-paste.desktop DESTINATION ${SERVICES_INSTALL_DIR})
