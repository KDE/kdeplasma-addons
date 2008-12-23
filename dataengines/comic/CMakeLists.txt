project(plasma_comic_dataengine)
set(comic_engine_SRCS
    cachedprovider.cpp
    comic.cpp
)

kde4_add_plugin(plasma_engine_comic ${comic_engine_SRCS})
target_link_libraries(plasma_engine_comic plasmacomicprovidercore ${KDE4_KDEUI_LIBS} ${KDE4_SOLID_LIBS} ${KDE4_PLASMA_LIBS})

install(TARGETS plasma_engine_comic DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES plasma-dataengine-comic.desktop DESTINATION ${SERVICES_INSTALL_DIR} )

########### plugin core library ############

set(comic_provider_core_SRCS
  comicprovider.cpp
)

kde4_add_library( plasmacomicprovidercore SHARED ${comic_provider_core_SRCS} )
target_link_libraries( plasmacomicprovidercore ${KDE4_KDEUI_LIBS} ${KDE4_KIO_LIBS} ${KDE4_KROSSCORE_LIBS} )
target_link_libraries( plasmacomicprovidercore LINK_INTERFACE_LIBRARIES ${KDE4_KDECORE_LIBS})
set_target_properties(
   plasmacomicprovidercore PROPERTIES VERSION 1.0.0 SOVERSION 1
)

install( TARGETS plasmacomicprovidercore ${INSTALL_TARGETS_DEFAULT_ARGS} )
install( FILES plasma_comicprovider.desktop DESTINATION ${SERVICETYPES_INSTALL_DIR} )

########### kross ###############

set( plasma_comic_krossprovider_SRCS
  comicproviderkross.cpp
  comicproviderwrapper.cpp
  comic_package.cpp
)

kde4_add_plugin( plasma_comic_krossprovider ${plasma_comic_krossprovider_SRCS} )
target_link_libraries( plasma_comic_krossprovider plasmacomicprovidercore ${KDE4_KDECORE_LIBS} ${QT_QTGUI_LIBRARY} ${KDE4_KIO_LIBS} ${KDE4_KROSSCORE_LIBS} ${KDE4_PLASMA_LIBS} )
install( TARGETS plasma_comic_krossprovider DESTINATION ${PLUGIN_INSTALL_DIR} )

set(comicpackage_SRCS
    comic_package.cpp
    comic_package_plugin.cpp
)

kde4_add_plugin( plasma_packagestructure_comic ${comicpackage_SRCS} )
target_link_libraries( plasma_packagestructure_comic ${KDE4_KDEUI_LIBS} ${KDE4_PLASMA_LIBS} )
install( TARGETS plasma_packagestructure_comic DESTINATION ${PLUGIN_INSTALL_DIR} )
install( FILES plasma-packagestructure-comic.desktop DESTINATION ${SERVICES_INSTALL_DIR} )
