add_definitions(-DTRANSLATION_DOMAIN=\"plasma_applet_org.kde.plasma.comic\")

add_subdirectory(engine)

plasma_add_applet(org.kde.plasma.comic
    QML_SOURCES
        qml/ButtonBar.qml
        qml/ComicBottomInfo.qml
        qml/ComicCentralView.qml
        qml/configBehavior.qml
        qml/configComics.qml
        qml/FullViewWidget.qml
        qml/ImageWidget.qml
        qml/main.qml
        qml/config.qml
    CPP_SOURCES
        comic.cpp
        comicmodel.cpp
        checknewstrips.cpp
        comicdata.cpp
        stripselector.cpp
    RESOURCES
        main.xml
)

target_link_libraries(org.kde.plasma.comic PRIVATE
    Qt::Gui
    Qt::Widgets
    Plasma::Plasma
    KF6::I18n
    KF6::WidgetsAddons
    KF6::KIOGui
    plasma_engine_comic
)

install(FILES comic.knsrc DESTINATION ${KDE_INSTALL_KNSRCDIR})
