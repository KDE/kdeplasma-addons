# A name for the project
project(plasma-translator)
 
# Find the required Libaries
find_package(KDE4 REQUIRED)
include(KDE4Defaults)
 
add_definitions (${QT_DEFINITIONS} ${KDE4_DEFINITIONS})
include_directories(
   ${CMAKE_SOURCE_DIR}
   ${CMAKE_BINARY_DIR}
   ${KDE4_INCLUDES}
   )
 
# We add our source code here
set(translator_engine_SRCS translator-engine.cpp translator.cpp translatorpackage.cpp)
 
# Now make sure all files get to the right place
kde4_add_plugin(plasma_engine_translator ${translator_engine_SRCS})
target_link_libraries(plasma_engine_translator ${KDE4_KDECORE_LIBS} ${KDE4_PLASMA_LIBS} ${KDE4_KDEUI_LIBS} ${KDE4_KROSSUI_LIBS} ${KDE4_KIO_LIBS} ${KDE4_KROSSCORE_LIBS})


install(TARGETS plasma_engine_translator
        DESTINATION ${PLUGIN_INSTALL_DIR})

install(FILES plasma-engine-translator.desktop
        DESTINATION ${SERVICES_INSTALL_DIR})

# install( FILES plasma-translationprovider.desktop DESTINATION ${SERVICETYPES_INSTALL_DIR} )

set(translatorpackage_SRC translatorpackage.cpp translatorpackage-plugin.cpp)

kde4_add_plugin( plasma_packagestructure_translator ${translatorpackage_SRC} )
target_link_libraries( plasma_packagestructure_translator ${KDE4_KDEUI_LIBS} ${KDE4_PLASMA_LIBS} )
install( TARGETS plasma_packagestructure_translator DESTINATION ${PLUGIN_INSTALL_DIR} )
install( FILES plasma-packagestructure-translator.desktop DESTINATION ${SERVICES_INSTALL_DIR} )
