project( plasma-bookmarks )

set( bookmarks_SRCS
    generalconfigeditor.cpp
    bookmarksplasmoid.cpp
    main.cpp
)

kde4_add_plugin( plasma_applet_bookmarks  ${bookmarks_SRCS})

target_link_libraries( plasma_applet_bookmarks
    ${KDE4_PLASMA_LIBS}
    ${KDE4_KDEUI_LIBS}
    ${KDE4_KIO_LIBS}
)

install( TARGETS plasma_applet_bookmarks  DESTINATION ${PLUGIN_INSTALL_DIR})
install( FILES plasma-applet-bookmarks.desktop  DESTINATION ${SERVICES_INSTALL_DIR} )
