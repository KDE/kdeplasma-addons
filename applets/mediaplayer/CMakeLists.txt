project(plasma-mediaplayer)

# We add our source code here
set(mediaplayer_SRCS
    mediaplayer.cpp
    dbus/playerdbushandler.cpp
    dbus/rootdbushandler.cpp
    dbus/tracklistdbushandler.cpp
)

QT4_ADD_DBUS_ADAPTOR(mediaplayer_SRCS dbus/org.freedesktop.MediaPlayer.player.xml
                     dbus/playerdbushandler.h PlayerDBusHandler playeradaptor PlayerAdaptor)
QT4_ADD_DBUS_ADAPTOR(mediaplayer_SRCS dbus/org.freedesktop.MediaPlayer.root.xml
                     dbus/rootdbushandler.h RootDBusHandler rootadaptor RootAdaptor)
QT4_ADD_DBUS_ADAPTOR(mediaplayer_SRCS dbus/org.freedesktop.MediaPlayer.tracklist.xml
                     dbus/tracklistdbushandler.h TrackListDBusHandler tracklistadaptor TrackListAdaptor)

#DBus interface for operations not supported by org.mpris stuff
QT4_ADD_DBUS_ADAPTOR(mediaplayer_SRCS org.kde.PlasmaMediaPlayer.xml
                     mediaplayer.h MediaPlayer)

# Now make sure all files get to the right place
kde4_add_plugin(plasma_applet_mediaplayer ${mediaplayer_SRCS})
target_link_libraries(plasma_applet_mediaplayer 
                      ${KDE4_PHONON_LIBS}
                      ${KDE4_PLASMA_LIBS} ${KDE4_KDEUI_LIBS} ${KDE4_KIO_LIBS} ${QT_QTDBUS_LIBRARY})



install(TARGETS plasma_applet_mediaplayer
        DESTINATION ${PLUGIN_INSTALL_DIR})

install(FILES plasma-applet-mediaplayer.desktop
        DESTINATION ${SERVICES_INSTALL_DIR})
