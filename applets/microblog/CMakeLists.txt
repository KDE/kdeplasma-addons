project(plasma-microblog)
include_directories(${CMAKE_CURRENT_SOURCE_DIR} )

set(microblog_SRCS
    microblog.cpp)
kde4_add_ui_files(microblog_SRCS configuration.ui)

kde4_add_plugin(plasma_applet_microblog ${microblog_SRCS})
target_link_libraries(plasma_applet_microblog ${KDE4_PLASMA_LIBS} ${KDE4_KIO_LIBS})

install(TARGETS plasma_applet_microblog DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES plasma-applet-microblog.desktop DESTINATION ${SERVICES_INSTALL_DIR})

install(FILES
    microblog.svgz
    DESTINATION ${DATA_INSTALL_DIR}/desktoptheme/default/widgets/)

