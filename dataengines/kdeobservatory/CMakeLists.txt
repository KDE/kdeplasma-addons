project(plasma-kdeobservatoryengine)
 
find_package(KDE4 REQUIRED)
include(KDE4Defaults)
 
add_definitions (${QT_DEFINITIONS} ${KDE4_DEFINITIONS})

include_directories(
   ${CMAKE_SOURCE_DIR}
   ${CMAKE_BINARY_DIR}
   ${KDE4_INCLUDES}
   )
 
set(kdeobservatory_engine_SRCS
    src/kdepresets.cpp
    src/kdeobservatoryengine.cpp
    src/kdeobservatoryservice.cpp
)
 
kde4_add_plugin(plasma_engine_kdeobservatory ${kdeobservatory_engine_SRCS})

target_link_libraries(plasma_engine_kdeobservatory
                      ${QT_QTNETWORK_LIBRARY}
                      ${KDE4_KDECORE_LIBS}
                      ${KDE4_KIO_LIBS}
                      ${KDE4_SOLID_LIBS}
                      ${KDE4_PLASMA_LIBS})
 
install(TARGETS plasma_engine_kdeobservatory
        DESTINATION ${PLUGIN_INSTALL_DIR})
 
install(FILES plasma-engine-kdeobservatory.desktop
        DESTINATION ${SERVICES_INSTALL_DIR})

install(FILES src/kdeobservatory.operations
        DESTINATION ${DATA_INSTALL_DIR}/plasma/services)
