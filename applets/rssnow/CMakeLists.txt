project(plasma-rssnow)
set(news_SRCS
    news.cpp
    header.cpp
    single-feed.cpp
    scroller.cpp
)

kde4_add_ui_files(news_SRCS config.ui feedsConfig.ui)

kde4_add_plugin(plasma_applet_rssnow ${news_SRCS})
target_link_libraries(plasma_applet_rssnow ${KDE4_PLASMA_LIBS} ${KDE4_KIO_LIBS} ${KDE4_KDEUI_LIBS})

install(TARGETS plasma_applet_rssnow DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES plasma-applet-rssnow.desktop DESTINATION ${SERVICES_INSTALL_DIR})
install(FILES feeds DESTINATION ${DATA_INSTALL_DIR}/rssnow)
install(FILES
arrows.svgz
rssnow.svgz
background.svgz
DESTINATION ${DATA_INSTALL_DIR}/desktoptheme/default/rssnow/)
