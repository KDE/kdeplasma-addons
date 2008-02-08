project(plasma_comic_dataengine)

set(comic_engine_SRCS
    cachedprovider.cpp
    comic.cpp
#    snoopyprovider.cpp
#    userfriendlyprovider.cpp
#    xkcdprovider.cpp
#    osnewsprovider.cpp
)

kde4_add_plugin(plasma_engine_comic ${comic_engine_SRCS})
target_link_libraries(plasma_engine_comic plasmacomicprovidercore ${KDE4_KDEUI_LIBS} ${PLASMA_LIBS})

install(TARGETS plasma_engine_comic DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES plasma-dataengine-comic.desktop DESTINATION ${SERVICES_INSTALL_DIR} )

########### plugin core library ############

set(comic_provider_core_SRCS
  comicprovider.cpp
)

kde4_add_library( plasmacomicprovidercore SHARED ${comic_provider_core_SRCS} )
target_link_libraries( plasmacomicprovidercore ${KDE4_KDEUI_LIBS} )
set_target_properties( plasmacomicprovidercore PROPERTIES VERSION 1.0.0 SOVERSION 1 )

install( TARGETS plasmacomicprovidercore DESTINATION ${LIB_INSTALL_DIR} )
install( FILES plasma_comicprovider.desktop DESTINATION ${SERVICETYPES_INSTALL_DIR} )


########### plugins ###############

set( comic_dilbert_provider_SRCS
     dilbertprovider.cpp
)

kde4_add_plugin( plasma_comic_dilbertprovider ${comic_dilbert_provider_SRCS} )
target_link_libraries( plasma_comic_dilbertprovider plasmacomicprovidercore )
install( TARGETS plasma_comic_dilbertprovider DESTINATION ${PLUGIN_INSTALL_DIR} )
install( FILES dilbertprovider.desktop DESTINATION ${SERVICES_INSTALL_DIR} )

set( comic_userfriendly_provider_SRCS
     userfriendlyprovider.cpp
)

kde4_add_plugin( plasma_comic_userfriendlyprovider ${comic_userfriendly_provider_SRCS} )
target_link_libraries( plasma_comic_userfriendlyprovider plasmacomicprovidercore )
install( TARGETS plasma_comic_userfriendlyprovider DESTINATION ${PLUGIN_INSTALL_DIR} )
install( FILES userfriendlyprovider.desktop DESTINATION ${SERVICES_INSTALL_DIR} )

set( comic_garfield_provider_SRCS
     garfieldprovider.cpp
)

kde4_add_plugin( plasma_comic_garfieldprovider ${comic_garfield_provider_SRCS} )
target_link_libraries( plasma_comic_garfieldprovider plasmacomicprovidercore )
install( TARGETS plasma_comic_garfieldprovider DESTINATION ${PLUGIN_INSTALL_DIR} )
install( FILES garfieldprovider.desktop DESTINATION ${SERVICES_INSTALL_DIR} )

set( comic_osnews_provider_SRCS
     osnewsprovider.cpp
)

kde4_add_plugin( plasma_comic_osnewsprovider ${comic_osnews_provider_SRCS} )
target_link_libraries( plasma_comic_osnewsprovider plasmacomicprovidercore ${KDE4_SYNDICATION_LIBS} )
install( TARGETS plasma_comic_osnewsprovider DESTINATION ${PLUGIN_INSTALL_DIR} )
install( FILES osnewsprovider.desktop DESTINATION ${SERVICES_INSTALL_DIR} )

set( comic_snoopy_provider_SRCS
     snoopyprovider.cpp
)

kde4_add_plugin( plasma_comic_snoopyprovider ${comic_snoopy_provider_SRCS} )
target_link_libraries( plasma_comic_snoopyprovider plasmacomicprovidercore ${KDE4_SYNDICATION_LIBS} )
install( TARGETS plasma_comic_snoopyprovider DESTINATION ${PLUGIN_INSTALL_DIR} )
install( FILES snoopyprovider.desktop DESTINATION ${SERVICES_INSTALL_DIR} )

set( comic_xkcd_provider_SRCS
     xkcdprovider.cpp
)

kde4_add_plugin( plasma_comic_xkcdprovider ${comic_xkcd_provider_SRCS} )
target_link_libraries( plasma_comic_xkcdprovider plasmacomicprovidercore ${KDE4_SYNDICATION_LIBS} )
install( TARGETS plasma_comic_xkcdprovider DESTINATION ${PLUGIN_INSTALL_DIR} )
install( FILES xkcdprovider.desktop DESTINATION ${SERVICES_INSTALL_DIR} )
