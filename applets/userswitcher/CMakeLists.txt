# SPDX-FileCopyrightText: 2025 Tobias Fella <tobias.fella@kde.org>
# SPDX-License-Identifier: BSD-3-Clause

plasma_add_applet(org.kde.plasma.userswitcher
    QML_SOURCES
        qml/ActionListDelegate.qml
        qml/configGeneral.qml
        qml/config.qml
        qml/ListDelegate.qml
        qml/main.qml
        qml/UserListDelegate.qml
    RESOURCES
        main.xml
    GENERATE_APPLET_CLASS
)
