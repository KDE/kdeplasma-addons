find_package(KDE4 REQUIRED)
include(KDE4Defaults)

add_definitions(${QT_DEFINITIONS} ${KDE4_DEFINITIONS})
include_directories(
   ${CMAKE_BINARY_DIR}
   ${KDE4_INCLUDE_DIR}
   ${KDE4_INCLUDE_DIR}/KDE
)

set(kopeterunner_SRCS
    kopeterunner.cpp
)

kde4_add_plugin(krunner_kopete
    ${kopeterunner_SRCS}
)
target_link_libraries(krunner_kopete
    ${QT4_QTCORE_LIBRARY}
    ${KDE4_PLASMA_LIBRARY}
)

install(
    TARGETS
        krunner_kopete
    DESTINATION
        ${PLUGIN_INSTALL_DIR}
)

install(
    FILES
        plasma-runner-kopete.desktop
    DESTINATION
        ${SERVICES_INSTALL_DIR}
)
