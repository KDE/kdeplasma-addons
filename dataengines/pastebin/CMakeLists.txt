project(pastebinengine)

set(pastebin_engine_SRCS
        backends/server.cpp
        backends/pastebinca.cpp
        backends/pastebincom.cpp
        backends/imagebinca.cpp
        backends/imageshack.cpp
        backends/simplestimagehosting.cpp
        pastebinengine.cpp
        pastebinservice.cpp)

kde4_add_plugin(plasma_engine_pastebin ${pastebin_engine_SRCS})
target_link_libraries(plasma_engine_pastebin
                      ${KDE4_KDECORE_LIBS}
                      ${KDE4_PLASMA_LIBS}
                      ${KDE4_KIO_LIBS})

install(TARGETS plasma_engine_pastebin
        DESTINATION ${PLUGIN_INSTALL_DIR})

install(FILES plasma-engine-pastebin.desktop
        DESTINATION ${SERVICES_INSTALL_DIR})

install(FILES pastebin.operations
        DESTINATION ${DATA_INSTALL_DIR}/plasma/services)
