set(potd_engine_SRCS
	cachedprovider.cpp
	potd.cpp
)

kde4_add_plugin(plasma_engine_potd ${potd_engine_SRCS} )
target_link_libraries(plasma_engine_potd plasmapotdprovidercore ${KDE4_KDEUI_LIBS} ${KDE4_PLASMA_LIBS} )

install(TARGETS plasma_engine_potd DESTINATION ${PLUGIN_INSTALL_DIR} )
install(FILES plasma-dataengine-potd.desktop DESTINATION ${SERVICES_INSTALL_DIR} )


########### plugin core library ############

set(potd_provider_core_SRCS
	potdprovider.cpp
)

kde4_add_library( plasmapotdprovidercore SHARED ${potd_provider_core_SRCS} )
target_link_libraries( plasmapotdprovidercore ${KDE4_KDEUI_LIBS} )
set_target_properties( plasmapotdprovidercore PROPERTIES VERSION 1.0.0 SOVERSION 1 )

install( TARGETS plasmapotdprovidercore ${INSTALL_TARGETS_DEFAULT_ARGS} )
install( FILES plasma_potdprovider.desktop DESTINATION ${SERVICETYPES_INSTALL_DIR} )


########### plugins ###############

set( potd_flickr_provider_SRCS
	flickrprovider.cpp
)

kde4_add_plugin( plasma_potd_flickrprovider ${potd_flickr_provider_SRCS} )
target_link_libraries( plasma_potd_flickrprovider plasmapotdprovidercore ${KDE4_KIO_LIBS}  ${KDE4_KDECORE_LIBS} ${QT_QTGUI_LIBRARY})
install( TARGETS plasma_potd_flickrprovider DESTINATION ${PLUGIN_INSTALL_DIR} )
install( FILES flickrprovider.desktop DESTINATION ${SERVICES_INSTALL_DIR} )


set( potd_apod_provider_SRCS
	apodprovider.cpp
)

kde4_add_plugin( plasma_potd_apodprovider ${potd_apod_provider_SRCS} )
target_link_libraries( plasma_potd_apodprovider plasmapotdprovidercore ${KDE4_KIO_LIBS} ${QT_QTGUI_LIBRARY} ${KDE4_KDECORE_LIBS})
install( TARGETS plasma_potd_apodprovider DESTINATION ${PLUGIN_INSTALL_DIR} )
install( FILES apodprovider.desktop DESTINATION ${SERVICES_INSTALL_DIR} )


set( potd_epod_provider_SRCS
	epodprovider.cpp
)

kde4_add_plugin( plasma_potd_epodprovider ${potd_epod_provider_SRCS} )
target_link_libraries( plasma_potd_epodprovider plasmapotdprovidercore ${KDE4_KIO_LIBS} ${KDE4_KDECORE_LIBS} ${QT_QTGUI_LIBRARY})
install( TARGETS plasma_potd_epodprovider DESTINATION ${PLUGIN_INSTALL_DIR} )
install( FILES epodprovider.desktop DESTINATION ${SERVICES_INSTALL_DIR} )


set( potd_osei_provider_SRCS
	oseiprovider.cpp
)

kde4_add_plugin( plasma_potd_oseiprovider ${potd_osei_provider_SRCS} )
target_link_libraries( plasma_potd_oseiprovider plasmapotdprovidercore ${KDE4_KIO_LIBS} ${KDE4_KDECORE_LIBS} ${QT_QTGUI_LIBRARY} )
install( TARGETS plasma_potd_oseiprovider DESTINATION ${PLUGIN_INSTALL_DIR} )
install( FILES oseiprovider.desktop DESTINATION ${SERVICES_INSTALL_DIR} )


set( potd_wcpotd_provider_SRCS
	wcpotdprovider.cpp
)

kde4_add_plugin( plasma_potd_wcpotdprovider ${potd_wcpotd_provider_SRCS} )
target_link_libraries( plasma_potd_wcpotdprovider plasmapotdprovidercore ${KDE4_KIO_LIBS} ${KDE4_KDECORE_LIBS} ${QT_QTGUI_LIBRARY} )
install( TARGETS plasma_potd_wcpotdprovider DESTINATION ${PLUGIN_INSTALL_DIR} )
install( FILES wcpotdprovider.desktop DESTINATION ${SERVICES_INSTALL_DIR} )
