project(plasma-twitter)
include_directories(${CMAKE_CURRENT_SOURCE_DIR} )

set(twitter_SRCS
    twitter.cpp)
kde4_add_ui_files(twitter_SRCS configuration.ui)

kde4_add_plugin(plasma_applet_twitter ${twitter_SRCS})
target_link_libraries(plasma_applet_twitter ${KDE4_PLASMA_LIBS} ${KDE4_KIO_LIBS})

install(TARGETS plasma_applet_twitter DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES plasma-twitter-default.desktop DESTINATION ${SERVICES_INSTALL_DIR})

install(FILES
    twitter.svgz
    DESTINATION ${DATA_INSTALL_DIR}/desktoptheme/default/widgets/)

