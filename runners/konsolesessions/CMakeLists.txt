add_definitions(-DTRANSLATION_DOMAIN="plasma_runner_konsolesessions")

set(krunner_konsolesessions_SRCS
    konsolesessions.cpp
)

add_library(krunner_konsolesessions MODULE ${krunner_konsolesessions_SRCS})
target_link_libraries(krunner_konsolesessions
    KF5::Runner
    KF5::I18n
)

install(TARGETS krunner_konsolesessions DESTINATION ${KDE_INSTALL_PLUGINDIR})

install(FILES konsolesessions.desktop DESTINATION ${KDE_INSTALL_KSERVICES5DIR})
