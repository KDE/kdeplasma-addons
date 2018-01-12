add_definitions(
    -DQT_USE_QSTRINGBUILDER
    -DQT_NO_CAST_TO_ASCII
    -DQT_NO_CAST_FROM_ASCII
    -DQT_NO_CAST_FROM_BYTEARRAY
)

set(plasmaweather_LIB_SRCS
    weatherpopupapplet.cpp
    weathervalidator.cpp
    weatherlocation.cpp
)

set(plasmaweather_HEADERS
    weatherpopupapplet.h
    weathervalidator.h
    weatherlocation.h
    plasmaweather_export.h
)

add_library(plasmaweather SHARED ${plasmaweather_LIB_SRCS})
generate_export_header(plasmaweather)

target_link_libraries(plasmaweather
    KF5::Plasma
    KF5::UnitConversion
    KF5::WidgetsAddons
    KF5::Notifications
    KF5::I18n
)
set_target_properties(plasmaweather PROPERTIES
    OUTPUT_NAME plasmaweatherprivate
)

install(TARGETS plasmaweather ${KDE_INSTALL_TARGETS_DEFAULT_ARGS})
# nothing outside of kdeplasma-addons uses this library, and the API is not stable -> do not install headers!
#install(FILES ${plasmaweather_HEADERS} DESTINATION ${INCLUDE_INSTALL_DIR}/plasmaweather COMPONENT Devel)
