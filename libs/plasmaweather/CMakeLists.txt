set(plasmaweather_LIB_SRCS
    weatherpopupapplet.cpp
    weathervalidator.cpp
    weatherlocation.cpp
    weatherconfig.cpp
)

set(plasmaweather_HEADERS
    weatherpopupapplet.h
    weathervalidator.h
    weatherconfig.h
    weatherlocation.h
    plasmaweather_export.h
)

ki18n_wrap_ui(plasmaweather_LIB_SRCS
    weatherconfig.ui
)

add_library(plasmaweather SHARED ${plasmaweather_LIB_SRCS})

target_link_libraries(plasmaweather
    KF5::Plasma
    KF5::UnitConversion
    KF5::ConfigWidgets
    KF5::Completion
    KF5::NewStuff
    KF5::I18n
)
set_target_properties(plasmaweather PROPERTIES VERSION ${GENERIC_LIB_VERSION} SOVERSION ${GENERIC_LIB_SOVERSION})
install(TARGETS plasmaweather ${INSTALL_TARGETS_DEFAULT_ARGS})
# nothing outside of kdeplasma-addons uses this library, and the API is not stable -> do not install headers!
#install(FILES ${plasmaweather_HEADERS} DESTINATION ${INCLUDE_INSTALL_DIR}/plasmaweather COMPONENT Devel)

install(FILES plasmaweather.knsrc DESTINATION ${CONFIG_INSTALL_DIR})
