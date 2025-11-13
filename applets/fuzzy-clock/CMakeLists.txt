# SPDX-FileCopyrightText: 2025 Tobias Fella <tobias.fella@kde.org>
# SPDX-License-Identifier: BSD-3-Clause

plasma_add_applet(org.kde.plasma.fuzzyclock
    QML_SOURCES
        qml/configAppearance.qml
        qml/config.qml
        qml/FuzzyClock.qml
        qml/main.qml
    RESOURCES
        main.xml
    GENERATE_APPLET_CLASS
)
