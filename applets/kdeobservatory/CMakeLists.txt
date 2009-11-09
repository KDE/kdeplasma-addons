project(plasma-kdeobservatory)
 
find_package(KDE4 REQUIRED)
include(KDE4Defaults)
 
add_definitions (${QT_DEFINITIONS} ${KDE4_DEFINITIONS})
include_directories(
   ${CMAKE_SOURCE_DIR}
   ${CMAKE_BINARY_DIR}
   ${KDE4_INCLUDES}
)
 
set(kdeobservatory_SRCS
    src/icollector.cpp
    src/iviewprovider.cpp
    src/kdeobservatory.cpp
    src/commitcollector.cpp
    src/topcommitersview.cpp
    src/topactiveprojectsview.cpp
    src/kdeobservatorydatabase.cpp
    src/kdeobservatoryconfiggeneral.cpp
    src/kdeobservatoryconfigprojects.cpp
)

kde4_add_ui_files(kdeobservatory_SRCS
		  src/ui/kdeobservatoryconfiggeneral.ui
		  src/ui/kdeobservatoryconfigproject.ui
		  src/ui/kdeobservatoryconfigprojects.ui
		  src/ui/kdeobservatoryconfigtopactiveprojects.ui
)
 
kde4_add_plugin(plasma_applet_kdeobservatory ${kdeobservatory_SRCS})
target_link_libraries(plasma_applet_kdeobservatory 
                      ${KDE4_PLASMA_LIBS} ${KDE4_KIO_LIBS} ${KDE4_KDEUI_LIBS} ${QT_QTSQL_LIBRARY})
 
install(TARGETS plasma_applet_kdeobservatory
        DESTINATION ${PLUGIN_INSTALL_DIR})
 
install(FILES plasma-applet-kdeobservatory.desktop
        DESTINATION ${SERVICES_INSTALL_DIR})

add_subdirectory(db)
