# Project Needs a name ofcourse
project(UnitConverter)

include_directories(
   ${CMAKE_SOURCE_DIR}
   ${CMAKE_BINARY_DIR}
   ${KDE4_INCLUDES}
   ${CONVERSION_INCLUDES}
)

# We add our source code here
set(unitconverter_SRCS unitconverter.cpp)

# Now make sure all files get to the right place
kde4_add_plugin(plasma_applet_unitconverter ${unitconverter_SRCS})
target_link_libraries(plasma_applet_unitconverter conversion
                      ${KDE4_PLASMA_LIBS} ${KDE4_KDEUI_LIBS})

install(TARGETS plasma_applet_unitconverter
        DESTINATION ${PLUGIN_INSTALL_DIR})

install(FILES plasma-applet-unitconverter.desktop
        DESTINATION ${SERVICES_INSTALL_DIR})

