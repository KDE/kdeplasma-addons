# SPDX-FileCopyrightText: 2022 Fushan Wen <qydwhotmail@gmail.com>

# SPDX-License-Identifier: GPL-2.0-or-later

ecm_add_qml_module(plasmacalendaralternatecalendarconfig URI org.kde.plasmacalendar.alternatecalendarconfig)
target_sources(plasmacalendaralternatecalendarconfig PRIVATE configplugin.cpp configstorage.cpp ../../calendarsystem.h)

target_link_libraries(plasmacalendaralternatecalendarconfig
    Qt::Qml
    Qt::Core
    KF6::ConfigCore
    KF6::I18n
)
ecm_finalize_qml_module(plasmacalendaralternatecalendarconfig)
