project(plasma-wallpaper-qml)

find_package(KDE4 REQUIRED)
find_package(KDeclarative REQUIRED)

include (KDE4Defaults)
include_directories(${KDE4_INCLUDE_DIR} ${QT_INCLUDES})
add_definitions (${KDE4_DEFINITIONS})

kde4_add_plugin(plasma_wallpaper_qml wallpapersqml.cpp)
target_link_libraries(plasma_wallpaper_qml
    ${QT_QTDECLARATIVE_LIBRARY}
    ${KDE4_PLASMA_LIBS} ${KDE4_KDECORE_LIBS} ${KDECLARATIVE_LIBRARIES}
)

install(TARGETS plasma_wallpaper_qml DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES plasma-wallpaper-qml.desktop DESTINATION ${SERVICES_INSTALL_DIR})

add_subdirectory(examples)
