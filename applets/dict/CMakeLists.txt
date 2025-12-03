find_package(Qt6WebEngineQuick ${QT_MIN_VERSION} CONFIG)

set_package_properties(Qt6WebEngineQuick PROPERTIES
    PURPOSE "Needed by the dict applet"
    URL "https://doc.qt.io/qt-6/qtwebengine-index.html"
    TYPE OPTIONAL
)

if (NOT Qt6WebEngineQuick_FOUND)
    return()
endif()

plasma_add_applet(org.kde.plasma_applet_dict GENERATE_APPLET_CLASS
    QML_SOURCES
        qml/AvailableDictSheet.qml
        qml/ConfigDictionaries.qml
        qml/config.qml
        qml/DictItemDelegate.qml
        qml/main.qml
    CPP_SOURCES
        dict_object.cpp
        dictionariesmodel.cpp
    RESOURCES
        main.xml
)
# TODO: fix pluginid "org.kde.plasma_applet_dict" into properly namespaced id "org.kde.plasma.dict"

target_link_libraries(org.kde.plasma_applet_dict PRIVATE
    KF6::I18n
    Qt::Quick
    Qt::WebEngineQuick
    engine_dict_static
)

add_definitions(-DTRANSLATION_DOMAIN="plasma_applet_org.kde.plasma_applet_dict")
