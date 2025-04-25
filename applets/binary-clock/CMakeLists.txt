# SPDX-FileCopyrightText: 2025 Tobias Fella <tobias.fella@kde.org>
# SPDX-License-Identifier: BSD-2-Clause

plasma_add_applet(org.kde.plasma.binaryclock
    QML_SOURCES
        BinaryClock.qml
        configGeneral.qml
        main.qml
        config.qml
    RESOURCES
        main.xml
    GENERATE_APPLET_CLASS
)
