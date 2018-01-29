add_definitions(-DTRANSLATION_DOMAIN="plasma_runner_mediawiki")

set(krunner_mediawiki_SRCS
    mediawikirunner.cpp
    mediawiki.cpp
)

add_library(krunner_mediawiki MODULE ${krunner_mediawiki_SRCS})
target_link_libraries(krunner_mediawiki
    KF5::Runner
    KF5::I18n
    Qt5::Network
)

install(TARGETS krunner_mediawiki DESTINATION ${KDE_INSTALL_PLUGINDIR} )

install(FILES
        plasma-runner-wikipedia.desktop
        plasma-runner-wikitravel.desktop
        plasma-runner-techbase.desktop
        plasma-runner-userbase.desktop
        DESTINATION ${KDE_INSTALL_KSERVICES5DIR}
)
