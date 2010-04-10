project(plasma-kdecommitsengine)
 
find_package(KDE4 REQUIRED)
include(KDE4Defaults)
 
add_definitions (${QT_DEFINITIONS} ${KDE4_DEFINITIONS})
include_directories(
   ${CMAKE_SOURCE_DIR}
   ${CMAKE_BINARY_DIR}
   ${KDE4_INCLUDES}
   )
 
set(kdecommits_engine_SRCS kdecommitsengine.cpp)
 
kde4_add_plugin(plasma_engine_kdecommits ${kdecommits_engine_SRCS})
target_link_libraries(plasma_engine_kdecommits
                      ${KDE4_KDECORE_LIBS}
                      ${KDE4_PLASMA_LIBS})
 
install(TARGETS plasma_engine_kdecommits
        DESTINATION ${PLUGIN_INSTALL_DIR})
 
install(FILES plasma-engine-kdecommits.desktop
        DESTINATION ${SERVICES_INSTALL_DIR})
