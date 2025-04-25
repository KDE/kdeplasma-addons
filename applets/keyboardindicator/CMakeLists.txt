# SPDX-FileCopyrightText: 2025 Tobias Fella <tobias.fella@kde.org>
# SPDX-License-Identifier: BSD-2-Clause

plasma_add_applet(org.kde.plasma.keyboardindicator
    QML_SOURCES
        main.qml
        config.qml
        configAppearance.qml
    RESOURCES
        main.xml
    GENERATE_APPLET_CLASS
)
