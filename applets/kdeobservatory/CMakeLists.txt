project(plasma-kdeobservatory)

set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules ${CMAKE_MODULE_PATH})
 
find_package(KDE4 REQUIRED)
find_package(Qwt REQUIRED)

include(KDE4Defaults)
 
add_definitions (${QT_DEFINITIONS} ${KDE4_DEFINITIONS})
include_directories(
   ${CMAKE_SOURCE_DIR}
   ${CMAKE_BINARY_DIR}
   ${KDE4_INCLUDES}
   ${Qwt5_INCLUDE_DIR}
)
 
set(kdeobservatory_SRCS
    src/iviewprovider.cpp
    src/kdeobservatory.cpp
    src/krazyreportview.cpp
    src/topdevelopersview.cpp
    src/commithistoryview.cpp
    src/topactiveprojectsview.cpp
    src/kdeobservatoryconfigviews.cpp
    src/kdeobservatoryconfiggeneral.cpp
    src/kdeobservatoryconfigprojects.cpp
)

kde4_add_ui_files(kdeobservatory_SRCS
		  src/ui/kdeobservatoryconfigviews.ui
		  src/ui/kdeobservatoryconfiggeneral.ui
		  src/ui/kdeobservatoryconfigproject.ui
		  src/ui/kdeobservatoryconfigprojects.ui
)
 
kde4_add_plugin(plasma_applet_kdeobservatory ${kdeobservatory_SRCS})
target_link_libraries(plasma_applet_kdeobservatory 
                      ${KDE4_PLASMA_LIBS} ${KDE4_KIO_LIBS} ${KDE4_KDEUI_LIBS} ${KDE4_SOLID_LIBS} ${QT_QTSQL_LIBRARY} ${Qwt5-Qt4_LIBRARY})
 
install(TARGETS plasma_applet_kdeobservatory
        DESTINATION ${PLUGIN_INSTALL_DIR})
 
install(FILES plasma-applet-kdeobservatory.desktop
        DESTINATION ${SERVICES_INSTALL_DIR})

add_subdirectory(icons)
