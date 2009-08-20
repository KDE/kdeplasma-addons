project(conversion)

set(conversion_LIB_SRCS
    converter.cpp
    value.cpp
    unit.cpp
    unitcategory.cpp
    length.cpp
    area.cpp
    volume.cpp
    temperature.cpp
    velocity.cpp
    mass.cpp
    pressure.cpp
    energy.cpp
    currency.cpp
    power.cpp
    timeunit.cpp
    fuel_efficiency.cpp
    conversioni18ncatalog.cpp
)

set(conversion_HEADERS
    converter.h
    value.h
    unit.h
    unitcategory.h
    plasmaconversion_export.h
)

kde4_add_library(conversion SHARED ${conversion_LIB_SRCS})
target_link_libraries(conversion ${KDE4_KIO_LIBS} ${KDE4_SOLID_LIBS})
set_target_properties(conversion PROPERTIES VERSION ${GENERIC_LIB_VERSION} SOVERSION ${GENERIC_LIB_SOVERSION})
install(TARGETS conversion ${INSTALL_TARGETS_DEFAULT_ARGS})
install(FILES ${conversion_HEADERS} DESTINATION ${INCLUDE_INSTALL_DIR}/conversion COMPONENT Devel)
install(FILES FindConversion.cmake DESTINATION ${DATA_INSTALL_DIR}/cmake/modules)