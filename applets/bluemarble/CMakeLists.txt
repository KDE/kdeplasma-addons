project(plasma-bluemarble)


set(bluemarble_SRCS
    bluemarble.cpp
    kwinglutils.cpp
    kwinglutils_funcs.cpp)

kde4_add_plugin(plasma_applet_bluemarble ${bluemarble_SRCS})
target_link_libraries(plasma_applet_bluemarble ${KDE4_PLASMA_LIBS} GLU ${KDE4_KDEUI_LIBS} ${QT_QTOPENGL_LIBRARY} ${OPENGL_gl_LIBRARY} ${OPENGL_glu_LIBRARY})


install(TARGETS plasma_applet_bluemarble DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES plasma-applet-bluemarble.desktop DESTINATION ${SERVICES_INSTALL_DIR})

install( FILES
    data/earth.png
    data/earth-night.png
    data/earth.frag
    data/earth.vert
    data/atmosphere.frag
    data/atmosphere.vert
    DESTINATION  ${DATA_INSTALL_DIR}/plasma-bluemarble )
