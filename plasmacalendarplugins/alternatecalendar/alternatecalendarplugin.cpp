/*
    SPDX-FileCopyrightText: 2022 Fushan Wen <qydwhotmail@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "config-ICU.h"

#include "alternatecalendarplugin.h"

#include <KSharedConfig>

#include "provider/qtcalendar.h"
#if HAVE_ICU
#include "provider/chinesecalendar.h"
#include "provider/hebrewcalendar.h"
#include "provider/indiancalendar.h"
#include "provider/islamiccalendar.h"
#endif

using SubLabel = CalendarEvents::CalendarEventsPlugin::SubLabel;

AlternateCalendarPlugin::AlternateCalendarPlugin(QObject *parent)
    : CalendarEvents::CalendarEventsPlugin(parent)
{
    m_subLabelsCache.setMaxCost(42 * 3 /*previous, current, next*/);

    auto config = KSharedConfig::openConfig(QStringLiteral("plasma_calendar_alternatecalendar"));
    m_generalConfigGroup = config->group("General");
    m_configWatcher = KConfigWatcher::create(config);
    connect(m_configWatcher.get(), &KConfigWatcher::configChanged, this, &AlternateCalendarPlugin::updateSettings);

    init();
}

AlternateCalendarPlugin::~AlternateCalendarPlugin()
{
}

void AlternateCalendarPlugin::loadEventsForDateRange(const QDate &startDate, const QDate &endDate)
{
    m_lastStartDate = startDate;
    m_lastEndDate = endDate;

    if (!endDate.isValid() || m_calendarSystem == CalendarSystem::Gregorian) {
        return;
    }

    QHash<QDate, QCalendar::YearMonthDay> alternateDatesData;
    QHash<QDate, CalendarEvents::CalendarEventsPlugin::SubLabel> subLabelsData;

    for (QDate date = startDate; date <= endDate && date.isValid(); date = date.addDays(1)) {
        const QDate offsetDate = date.addDays(m_dateOffset);
        const QCalendar::YearMonthDay alt = m_calendarProvider->fromGregorian(offsetDate);

        if (alt.day != date.day() || alt.month != date.month() || alt.year != date.year()) {
            alternateDatesData.insert(date, QCalendar::YearMonthDay(alt.year, alt.month, alt.day));
        }

        if (m_subLabelsCache.contains(date)) {
            subLabelsData.insert(date, *m_subLabelsCache.object(date));
        } else {
            const auto it = subLabelsData.insert(date, m_calendarProvider->subLabels(offsetDate));
            m_subLabelsCache.insert(date, new SubLabel(*it));
        }
    }

    if (alternateDatesData.size() > 0) {
        Q_EMIT alternateCalendarDateReady(alternateDatesData);
    }
    Q_EMIT subLabelReady(subLabelsData);
}

void AlternateCalendarPlugin::updateSettings()
{
    init();
    loadEventsForDateRange(m_lastStartDate, m_lastEndDate);
}

void AlternateCalendarPlugin::init()
{
    m_dateOffset = m_generalConfigGroup.readEntry("dateOffset", 0);

    // Find the matched calendar system
    const QString system = m_generalConfigGroup.readEntry("calendarSystem", "Julian");
    const auto systemIt = s_calendarMap.find(system);

    if (systemIt == s_calendarMap.end()) {
        // Invalid config, fall back to Gregorian
        m_calendarSystem = CalendarSystem::Gregorian;
    } else {
        m_calendarSystem = systemIt->second.system;
    }

    // Load/Reload the calendar provider
    switch (m_calendarSystem) {
#if HAVE_ICU
    case CalendarSystem::Chinese:
        m_calendarProvider.reset(new ChineseCalendarProvider(this, m_calendarSystem));
        break;
    case CalendarSystem::Indian:
        m_calendarProvider.reset(new IndianCalendarProvider(this, m_calendarSystem));
        break;
    case CalendarSystem::Hebrew:
        m_calendarProvider.reset(new HebrewCalendarProvider(this, m_calendarSystem));
        break;
    case CalendarSystem::Jalali:
    case CalendarSystem::Islamic:
    case CalendarSystem::IslamicCivil:
    case CalendarSystem::IslamicUmalqura:
        m_calendarProvider.reset(new IslamicCalendarProvider(this, m_calendarSystem));
        break;
#endif
#ifndef QT_BOOTSTRAPPED
    case CalendarSystem::Julian:
    case CalendarSystem::Milankovic:
        m_calendarProvider.reset(new QtCalendarProvider(this, m_calendarSystem));
        break;
#endif
    default:
        m_calendarProvider.reset(new AbstractCalendarProvider(this, m_calendarSystem));
    }

    // Clear the old cache when config is reloaded
    m_subLabelsCache.clear();
}

#include "moc_alternatecalendarplugin.cpp"