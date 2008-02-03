set(comic_engine_SRCS
    cachedprovider.cpp
    comic.cpp
    comicprovider.cpp
    dilbertprovider.cpp
    garfieldprovider.cpp
    snoopyprovider.cpp
    userfriendlyprovider.cpp
    xkcdprovider.cpp
    osnewsprovider.cpp
)

kde4_add_plugin(plasma_engine_comic ${comic_engine_SRCS})
target_link_libraries(plasma_engine_comic ${KDE4_KDEUI_LIBS} ${PLASMA_LIBS} ${KDE4_SYNDICATION_LIBS})

install(TARGETS plasma_engine_comic DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES plasma-dataengine-comic.desktop DESTINATION ${SERVICES_INSTALL_DIR} )
