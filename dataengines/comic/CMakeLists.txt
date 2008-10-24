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
target_link_libraries( plasmacomicprovidercore ${KDE4_KDEUI_LIBS} ${KDE4_KIO_LIBS} ${KDE4_KROSSCORE_LIBS} )
set_target_properties(
   plasmacomicprovidercore PROPERTIES VERSION 1.0.0 SOVERSION 1
   ${KDE4_DISABLE_PROPERTY_}LINK_INTERFACE_LIBRARIES "${KDE4_KDECORE_LIBS}"
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
target_link_libraries( plasma_comic_krossprovider plasmacomicprovidercore ${KDE4_KDECORE_LIBS} ${QT_QTGUI_LIBRARY} ${KDE4_KIO_LIBS} ${KDE4_KROSSCORE_LIBS} ${PLASMA_LIBS} )
install( TARGETS plasma_comic_krossprovider DESTINATION ${PLUGIN_INSTALL_DIR} )

set(comicpackage_SRCS
    comic_package.cpp
    comic_package_plugin.cpp
)

kde4_add_plugin( plasma_packagestructure_comic ${comicpackage_SRCS} )
target_link_libraries( plasma_packagestructure_comic plasma ${KDE4_KDEUI_LIBS} ${PLASMA_LIBS} )
install( TARGETS plasma_packagestructure_comic DESTINATION ${PLUGIN_INSTALL_DIR} )
install( FILES plasma-packagestructure-comic.desktop DESTINATION ${SERVICES_INSTALL_DIR} )

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

set( comic_malvados_provider_SRCS
     malvadosprovider.cpp
)

kde4_add_plugin( plasma_comic_malvadosprovider ${comic_malvados_provider_SRCS} )
target_link_libraries( plasma_comic_malvadosprovider plasmacomicprovidercore ${KDE4_KDECORE_LIBS} ${QT_QTGUI_LIBRARY} ${QT_QTNETWORK_LIBRARY} )
install( TARGETS plasma_comic_malvadosprovider DESTINATION ${PLUGIN_INSTALL_DIR} )
install( FILES malvadosprovider.desktop DESTINATION ${SERVICES_INSTALL_DIR} )
install( FILES pics/plasma_comic_malvados.png DESTINATION ${DATA_INSTALL_DIR}/plasma-comic )

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

set( comic_nichtlustig_provider_SRCS
     nichtlustigprovider.cpp
)

kde4_add_plugin( plasma_comic_nichtlustigprovider ${comic_nichtlustig_provider_SRCS} )
target_link_libraries( plasma_comic_nichtlustigprovider plasmacomicprovidercore ${QT_QTGUI_LIBRARY} ${KDE4_KIO_LIBS} )
install( TARGETS plasma_comic_nichtlustigprovider DESTINATION ${PLUGIN_INSTALL_DIR} )
install( FILES nichtlustigprovider.desktop DESTINATION ${SERVICES_INSTALL_DIR} )
install( FILES pics/plasma_comic_nichtlustig.png DESTINATION ${DATA_INSTALL_DIR}/plasma-comic )
