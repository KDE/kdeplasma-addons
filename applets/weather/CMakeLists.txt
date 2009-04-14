project(weatherapplet)

# ----------------- Needed for Extragear ------------------------

# search packages used by KDE
find_package(KDE4 REQUIRED)
include (KDE4Defaults)
include (MacroLibrary)
include(MacroOptionalAddSubdirectory)
include(MacroOptionalDependPackage)

add_definitions (${QT_DEFINITIONS} ${KDE4_DEFINITIONS} -DQT_STRICT_ITERATORS)
include_directories (${CMAKE_SOURCE_DIR} ${CMAKE_BINARY_DIR} ${KDE4_INCLUDES})

# local cmake modules
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_SOURCE_DIR}/cmake/modules)

# --------------- End ---------------------

set(weatherapplet_SRCS
    weatherapplet.cpp
    weatherview.cpp
    weatherdelegate.cpp)

kde4_add_plugin(plasma_applet_weather ${weatherapplet_SRCS})
target_link_libraries(plasma_applet_weather ${KDE4_PLASMA_LIBS} ${KDE4_KIO_LIBS} ${KDE4_KDEUI_LIBS} plasmaweather weather_ion ${QT_QTWEBKIT_LIBRARY})

install(TARGETS plasma_applet_weather DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES plasma-applet-weather.desktop DESTINATION ${SERVICES_INSTALL_DIR})

install(FILES 
        wind-arrows.svgz 
        DESTINATION ${DATA_INSTALL_DIR}/desktoptheme/default/weather/)
