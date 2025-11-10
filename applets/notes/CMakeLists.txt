add_definitions(-DTRANSLATION_DOMAIN="plasma_applet_org.kde.plasma.notes")

plasma_add_applet(org.kde.plasma.notes GENERATE_APPLET_CLASS
    QML_SOURCES
        qml/main.qml
        qml/ShortcutMenuItem.qml
        qml/configAppearance.qml
        qml/config.qml
    CPP_SOURCES
        abstractnoteloader.cpp
        documenthandler.cpp
        filesystemnoteloader.cpp
        note.cpp
        noteiconengine.cpp
        notemanager.cpp
        noteshelper.cpp
    RESOURCES
        main.xml
)

target_link_libraries(org.kde.plasma.notes PRIVATE
    Qt::Quick
    KF6::CoreAddons
    KF6::Svg
    KF6::KIOCore
)
