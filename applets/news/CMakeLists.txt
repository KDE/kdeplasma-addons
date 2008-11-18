project(plasma-news)

include_directories(${CMAKE_CURRENT_SOURCE_DIR} )

set(news_SRCS
    news.cpp
)

kde4_add_ui_files(news_SRCS config.ui feedsConfig.ui)

kde4_add_plugin(plasma_applet_news ${news_SRCS})
target_link_libraries(plasma_applet_news ${KDE4_PLASMA_LIBS} ${KDE4_KIO_LIBS} ${QT_QTWEBKIT_LIBRARY})

install(TARGETS plasma_applet_news DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES plasma-applet-news.desktop DESTINATION ${SERVICES_INSTALL_DIR})
install(FILES news.css DESTINATION ${DATA_INSTALL_DIR}/desktoptheme/default/stylesheets/)
