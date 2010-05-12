project(plasma-opendesktop)

set(opendesktop_SRCS
    actionstack.cpp
    loginwidget.cpp
    friendlist.cpp
    contactcontainer.cpp
    friendmanagementcontainer.cpp
    friendmanagementwidget.cpp
    messagecounter.cpp
    requestfriendshipwidget.cpp
    messagelist.cpp
    messagewatchlist.cpp
    messagewidget.cpp
    personwatch.cpp
    personwatchlist.cpp
    sourcewatchlist.cpp
    utils.cpp
    sendmessagewidget.cpp
    opendesktop.cpp
    contactwidget.cpp
    userwidget.cpp
    contactimage.cpp
    stylesheet.cpp
    contactlist.cpp
)

kde4_add_ui_files(opendesktop_SRCS opendesktopConfig.ui opendesktopLocationConfig.ui )
kde4_add_plugin(plasma_applet_opendesktop ${opendesktop_SRCS})

target_link_libraries(
    plasma_applet_opendesktop
    ${KDE4_KIO_LIBS}
    ${KDE4_PLASMA_LIBS}
    ${KDE4_KCMUTILS_LIBS}
    ${QT_QTWEBKIT_LIBRARY}
)

install(TARGETS plasma_applet_opendesktop DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES plasma-applet-opendesktop.desktop DESTINATION ${SERVICES_INSTALL_DIR})

install(FILES user.css DESTINATION ${DATA_INSTALL_DIR}/plasma-applet-opendesktop/)

