add_definitions(-DTRANSLATION_DOMAIN="plasma_applet_org.kde.plasma.diskquota")

plasma_add_applet(org.kde.plasma.diskquota
    QML_SOURCES
        qml/main.qml
        qml/ListDelegateItem.qml
    CPP_SOURCES
        DiskQuota.cpp
        QuotaListModel.cpp
        QuotaItem.cpp
    GENERATE_APPLET_CLASS
)

target_link_libraries(org.kde.plasma.diskquota PRIVATE
    Qt::Quick
    KF6::CoreAddons
    KF6::I18n
)
