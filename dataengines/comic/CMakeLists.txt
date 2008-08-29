project(plasma_comic_dataengine)
set(comic_engine_SRCS
    cachedprovider.cpp
    comic.cpp
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
target_link_libraries( plasmacomicprovidercore ${KDE4_KDEUI_LIBS} ${KDE4_KIO_LIBS} )
set_target_properties( 
   plasmacomicprovidercore PROPERTIES VERSION 1.0.0 SOVERSION 1
   ${KDE4_DISABLE_PROPERTY_}LINK_INTERFACE_LIBRARIES "${KDE4_KDECORE_LIBS}"
)

install( TARGETS plasmacomicprovidercore ${INSTALL_TARGETS_DEFAULT_ARGS} )
install( FILES plasma_comicprovider.desktop DESTINATION ${SERVICETYPES_INSTALL_DIR} )


########### plugins ###############

set( comic_dilbert_provider_SRCS
     dilbertprovider.cpp
)

kde4_add_plugin( plasma_comic_dilbertprovider ${comic_dilbert_provider_SRCS} )
target_link_libraries( plasma_comic_dilbertprovider plasmacomicprovidercore ${KDE4_KDECORE_LIBS} ${QT_QTGUI_LIBRARY} ${KDE4_KIO_LIBS})
install( TARGETS plasma_comic_dilbertprovider DESTINATION ${PLUGIN_INSTALL_DIR} )
install( FILES dilbertprovider.desktop DESTINATION ${SERVICES_INSTALL_DIR} )
install( FILES pics/plasma_comic_dilbert.png DESTINATION ${DATA_INSTALL_DIR}/plasma-comic )

#### next ####

set( comic_userfriendly_provider_SRCS
     userfriendlyprovider.cpp
)

kde4_add_plugin( plasma_comic_userfriendlyprovider ${comic_userfriendly_provider_SRCS} )
target_link_libraries( plasma_comic_userfriendlyprovider plasmacomicprovidercore ${KDE4_KDECORE_LIBS} ${QT_QTGUI_LIBRARY} ${KDE4_KIO_LIBS} )
install( TARGETS plasma_comic_userfriendlyprovider DESTINATION ${PLUGIN_INSTALL_DIR} )
install( FILES userfriendlyprovider.desktop DESTINATION ${SERVICES_INSTALL_DIR} )
install( FILES pics/plasma_comic_userfriendly.png DESTINATION ${DATA_INSTALL_DIR}/plasma-comic )

#### next ####

set( comic_garfield_provider_SRCS
     garfieldprovider.cpp
)

kde4_add_plugin( plasma_comic_garfieldprovider ${comic_garfield_provider_SRCS} )
target_link_libraries( plasma_comic_garfieldprovider plasmacomicprovidercore ${KDE4_KDECORE_LIBS} ${QT_QTGUI_LIBRARY} ${KDE4_KIO_LIBS})
install( TARGETS plasma_comic_garfieldprovider DESTINATION ${PLUGIN_INSTALL_DIR} )
install( FILES garfieldprovider.desktop DESTINATION ${SERVICES_INSTALL_DIR} )
install( FILES pics/plasma_comic_garfield.png DESTINATION ${DATA_INSTALL_DIR}/plasma-comic )

#### next ####
if( Boost_FOUND )
    include_directories(${Boost_INCLUDE_DIR})
    set( comic_osnews_provider_SRCS
         osnewsprovider.cpp
    )

    kde4_add_plugin( plasma_comic_osnewsprovider ${comic_osnews_provider_SRCS} )
    target_link_libraries( plasma_comic_osnewsprovider plasmacomicprovidercore ${KDE4_SYNDICATION_LIBS} )
    install( TARGETS plasma_comic_osnewsprovider DESTINATION ${PLUGIN_INSTALL_DIR} )
    install( FILES osnewsprovider.desktop DESTINATION ${SERVICES_INSTALL_DIR} )
    install( FILES pics/plasma_comic_osnews.png DESTINATION ${DATA_INSTALL_DIR}/plasma-comic )
endif( Boost_FOUND )
#### next ####

set( comic_snoopy_provider_SRCS
     snoopyprovider.cpp
)

kde4_add_plugin( plasma_comic_snoopyprovider ${comic_snoopy_provider_SRCS} )
target_link_libraries( plasma_comic_snoopyprovider plasmacomicprovidercore ${QT_QTGUI_LIBRARY} ${KDE4_KIO_LIBS} )
install( TARGETS plasma_comic_snoopyprovider DESTINATION ${PLUGIN_INSTALL_DIR} )
install( FILES snoopyprovider.desktop DESTINATION ${SERVICES_INSTALL_DIR} )
install( FILES pics/plasma_comic_snoopy.png DESTINATION ${DATA_INSTALL_DIR}/plasma-comic )

#### next ####

set( comic_xkcd_provider_SRCS
     xkcdprovider.cpp
)

kde4_add_plugin( plasma_comic_xkcdprovider ${comic_xkcd_provider_SRCS} )
target_link_libraries( plasma_comic_xkcdprovider plasmacomicprovidercore ${KDE4_KIO_LIBS} ${QT_QTGUI_LIBRARY} )
install( TARGETS plasma_comic_xkcdprovider DESTINATION ${PLUGIN_INSTALL_DIR} )
install( FILES xkcdprovider.desktop DESTINATION ${SERVICES_INSTALL_DIR} )
install( FILES pics/plasma_comic_xkcd.png DESTINATION ${DATA_INSTALL_DIR}/plasma-comic )

#### next ####

set( comic_phd_provider_SRCS
     phdprovider.cpp
)

kde4_add_plugin( plasma_comic_phdprovider ${comic_phd_provider_SRCS} )
target_link_libraries( plasma_comic_phdprovider plasmacomicprovidercore ${QT_QTGUI_LIBRARY} ${KDE4_KIO_LIBS} )
install( TARGETS plasma_comic_phdprovider DESTINATION ${PLUGIN_INSTALL_DIR} )
install( FILES phdprovider.desktop DESTINATION ${SERVICES_INSTALL_DIR} )
install( FILES pics/plasma_comic_phd.png DESTINATION ${DATA_INSTALL_DIR}/plasma-comic )

### next ####

set( comic_shithappens_provider_SRCS
     shithappensprovider.cpp
)

kde4_add_plugin( plasma_comic_shithappensprovider ${comic_shithappens_provider_SRCS} )
target_link_libraries( plasma_comic_shithappensprovider plasmacomicprovidercore ${QT_QTGUI_LIBRARY} ${KDE4_KIO_LIBS} )
install( TARGETS plasma_comic_shithappensprovider DESTINATION ${PLUGIN_INSTALL_DIR} )
install( FILES shithappensprovider.desktop DESTINATION ${SERVICES_INSTALL_DIR} )
install( FILES pics/plasma_comic_shithappens.png DESTINATION ${DATA_INSTALL_DIR}/plasma-comic )
install( FILES pics/plasma_comic_shithappens-header.png DESTINATION ${DATA_INSTALL_DIR}/plasma-comic )

### next ####

set( comic_cad_provider_SRCS
     cadprovider.cpp
)

kde4_add_plugin( plasma_comic_cadprovider ${comic_cad_provider_SRCS} )
target_link_libraries( plasma_comic_cadprovider plasmacomicprovidercore ${QT_QTGUI_LIBRARY} ${KDE4_KIO_LIBS} )
install( TARGETS plasma_comic_cadprovider DESTINATION ${PLUGIN_INSTALL_DIR} )
install( FILES cadprovider.desktop DESTINATION ${SERVICES_INSTALL_DIR} )
install( FILES pics/plasma_comic_cad.png DESTINATION ${DATA_INSTALL_DIR}/plasma-comic )

### next ####

set( comic_calvinandhobbes_provider_SRCS
     calvinandhobbesprovider.cpp
)

kde4_add_plugin( plasma_comic_calvinandhobbesprovider ${comic_calvinandhobbes_provider_SRCS} )
target_link_libraries( plasma_comic_calvinandhobbesprovider plasmacomicprovidercore ${QT_QTGUI_LIBRARY} ${KDE4_KIO_LIBS} )
install( TARGETS plasma_comic_calvinandhobbesprovider DESTINATION ${PLUGIN_INSTALL_DIR} )
install( FILES calvinandhobbesprovider.desktop DESTINATION ${SERVICES_INSTALL_DIR} )
install( FILES pics/plasma_comic_calvinandhobbes.png DESTINATION ${DATA_INSTALL_DIR}/plasma-comic )

### next ####

set( comic_nichtlustig_provider_SRCS
     nichtlustigprovider.cpp
)

kde4_add_plugin( plasma_comic_nichtlustigprovider ${comic_nichtlustig_provider_SRCS} )
target_link_libraries( plasma_comic_nichtlustigprovider plasmacomicprovidercore ${QT_QTGUI_LIBRARY} ${KDE4_KIO_LIBS} )
install( TARGETS plasma_comic_nichtlustigprovider DESTINATION ${PLUGIN_INSTALL_DIR} )
install( FILES nichtlustigprovider.desktop DESTINATION ${SERVICES_INSTALL_DIR} )
install( FILES pics/plasma_comic_nichtlustig.png DESTINATION ${DATA_INSTALL_DIR}/plasma-comic )

### next ####

set( comic_unshelved_provider_SRCS
     unshelvedprovider.cpp
)

kde4_add_plugin( plasma_comic_unshelvedprovider ${comic_unshelved_provider_SRCS} )
target_link_libraries( plasma_comic_unshelvedprovider plasmacomicprovidercore ${QT_QTGUI_LIBRARY} ${KDE4_KIO_LIBS} )
install( TARGETS plasma_comic_unshelvedprovider DESTINATION ${PLUGIN_INSTALL_DIR} )
install( FILES unshelvedprovider.desktop DESTINATION ${SERVICES_INSTALL_DIR} )
install( FILES pics/plasma_comic_unshelved.png DESTINATION ${DATA_INSTALL_DIR}/plasma-comic )

### next ####

set( comic_extralife_provider_SRCS
extralifeprovider.cpp
)

kde4_add_plugin( plasma_comic_extralifeprovider ${comic_extralife_provider_SRCS} )
target_link_libraries( plasma_comic_extralifeprovider plasmacomicprovidercore ${QT_QTGUI_LIBRARY} ${KDE4_KIO_LIBS} )
install( TARGETS plasma_comic_extralifeprovider DESTINATION ${PLUGIN_INSTALL_DIR} )
install( FILES extralifeprovider.desktop DESTINATION ${SERVICES_INSTALL_DIR} )
install( FILES pics/plasma_comic_extralife.png DESTINATION ${DATA_INSTALL_DIR}/plasma-comic )

### next ####

set( comic_doonesbury_provider_SRCS
doonesburyprovider.cpp
)

kde4_add_plugin( plasma_comic_doonesburyprovider ${comic_doonesbury_provider_SRCS} )
target_link_libraries( plasma_comic_doonesburyprovider plasmacomicprovidercore ${QT_QTGUI_LIBRARY} ${KDE4_KIO_LIBS} )
install( TARGETS plasma_comic_doonesburyprovider DESTINATION ${PLUGIN_INSTALL_DIR} )
install( FILES doonesburyprovider.desktop DESTINATION ${SERVICES_INSTALL_DIR} )
install( FILES pics/plasma_comic_doonesbury.png DESTINATION ${DATA_INSTALL_DIR}/plasma-comic )

### next ####

set( comic_boondocks_provider_SRCS
boondocksprovider.cpp
)

kde4_add_plugin( plasma_comic_boondocksprovider ${comic_boondocks_provider_SRCS} )
target_link_libraries( plasma_comic_boondocksprovider plasmacomicprovidercore ${QT_QTGUI_LIBRARY} ${KDE4_KIO_LIBS} )
install( TARGETS plasma_comic_boondocksprovider DESTINATION ${PLUGIN_INSTALL_DIR} )
install( FILES boondocksprovider.desktop DESTINATION ${SERVICES_INSTALL_DIR} )
install( FILES pics/plasma_comic_boondocks.png DESTINATION ${DATA_INSTALL_DIR}/plasma-comic )
