/*
    SPDX-FileCopyrightText: 2018 Friedrich W. H. Kossebau <kossebau@kde.org>
    SPDX-FileCopyrightText: 2022 Fushan Wen <qydwhotmail@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "../../config-ICU.h"
#include "configstorage.h"

#include <QMetaEnum>

#include <KLocalizedString>
#include <KSharedConfig>

CalendarSystemModel::CalendarSystemModel(QObject *parent)
    : QAbstractListModel(parent)
{
    const std::map<CalendarSystem::System, QString> textMap{
#ifndef QT_BOOTSTRAPPED
        {CalendarSystem::Julian, i18ndc("plasma_calendar_alternatecalendar", "@item:inlist", "Julian")},
        {CalendarSystem::Milankovic, i18ndc("plasma_calendar_alternatecalendar", "@item:inlist", "Milankovic")},
#endif
#if QT_CONFIG(jalalicalendar)
        {CalendarSystem::Jalali, i18ndc("plasma_calendar_alternatecalendar", "@item:inlist", "The Solar Hijri Calendar (Persian)")},
#endif
#if QT_CONFIG(islamiccivilcalendar)
        {CalendarSystem::IslamicCivil, i18ndc("plasma_calendar_alternatecalendar", "@item:inlist", "The Islamic Civil Calendar")},
#endif
#ifdef HAVE_ICU
        {CalendarSystem::Chinese, i18ndc("plasma_calendar_alternatecalendar", "@item:inlist", "Chinese Lunar Calendar")},
        {CalendarSystem::Indian, i18ndc("plasma_calendar_alternatecalendar", "@item:inlist", "Indian National Calendar")},
#endif
    };
    const QMetaEnum e = QMetaEnum::fromType<CalendarSystem::System>();

    beginInsertRows(QModelIndex(), 0, e.keyCount() - 2 /* Gregorian */);

    m_items.reserve(e.keyCount() - 1);
    for (int k = 0; k < e.keyCount(); k++) {
        CalendarSystemItem item;
        item.value = static_cast<CalendarSystem::System>(e.value(k));
        if (textMap.count(item.value) > 0) {
            item.text = textMap.at(item.value);
            m_items.emplace_back(item);
        }
    }

    endInsertRows();
}

QVariant CalendarSystemModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid()) {
        return QVariant();
    }

    const auto &item = m_items.at(index.row());

    switch (role) {
    case Qt::DisplayRole:
        return item.text;
    case ValueRole:
        return item.value;
    default:
        return QVariant();
    }
}

int CalendarSystemModel::rowCount(const QModelIndex &parent) const
{
    return parent.isValid() ? 0 : m_items.size();
}

QHash<int, QByteArray> CalendarSystemModel::roleNames() const
{
    return {
        {Qt::DisplayRole, QByteArrayLiteral("display")},
        {ValueRole, QByteArrayLiteral("value")},
    };
}

int CalendarSystemModel::indexOf(CalendarSystem::System value) const
{
    const auto it = std::find_if(m_items.cbegin(), m_items.cend(), [value](const CalendarSystemItem &item) {
        return item.value == value;
    });

    if (it != m_items.cend()) {
        return std::distance(m_items.cbegin(), it);
    }

    return 0;
}

ConfigStorage::ConfigStorage(QObject *parent)
    : QObject(parent)
    , m_calendarSystemModel(new CalendarSystemModel(this))
{
    auto config = KSharedConfig::openConfig(QStringLiteral("plasma_calendar_alternatecalendar"));
    m_generalConfigGroup = config->group("General");

    m_calendarSystem = static_cast<CalendarSystem::System>(m_generalConfigGroup.readEntry("calendarSystem", static_cast<int>(CalendarSystem::Julian)));
    m_dateOffset = m_generalConfigGroup.readEntry("dateOffset", 0);
}

QAbstractListModel *ConfigStorage::calendarSystemModel() const
{
    return m_calendarSystemModel;
}

int ConfigStorage::currentIndex() const
{
    return m_calendarSystemModel->indexOf(m_calendarSystem);
}

void ConfigStorage::save()
{
    m_generalConfigGroup.writeEntry("calendarSystem", static_cast<int>(m_calendarSystem), KConfigBase::Notify);
    m_generalConfigGroup.writeEntry("dateOffset", m_dateOffset, KConfigBase::Notify);

    m_generalConfigGroup.sync();
}
