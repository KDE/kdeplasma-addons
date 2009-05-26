set(krunner_contacts_SRCS
    contactsrunner.cpp
    imageiconengine.cpp
)

kde4_add_plugin(krunner_contacts ${krunner_contacts_SRCS})
target_link_libraries(krunner_contacts ${KDE4_KIO_LIBS} ${KDE4_PLASMA_LIBS}
${KDE4_KABC_LIBS})

install(TARGETS krunner_contacts DESTINATION ${PLUGIN_INSTALL_DIR} )

install(FILES plasma-runner-contacts.desktop DESTINATION ${SERVICES_INSTALL_DIR})

