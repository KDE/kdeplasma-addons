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
    KF5::I18n
)
# TODO: find vars which would bump the version on every release, given ABI is unstable
set_target_properties(plasmaweather PROPERTIES VERSION 2.0.0 SOVERSION 1)
install(TARGETS plasmaweather ${INSTALL_TARGETS_DEFAULT_ARGS})
# nothing outside of kdeplasma-addons uses this library, and the API is not stable -> do not install headers!
#install(FILES ${plasmaweather_HEADERS} DESTINATION ${INCLUDE_INSTALL_DIR}/plasmaweather COMPONENT Devel)

install(FILES plasmaweather.knsrc DESTINATION ${CONFIG_INSTALL_DIR})
