/*
    SPDX-FileCopyrightText: 2022 Fushan Wen <qydwhotmail@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "config-ICU.h"

#include "alternatecalendarplugin.h"

#include <QCache>

#include <KConfigGroup>
#include <KConfigWatcher>
#include <KSharedConfig>

#include "provider/qtcalendar.h"
#if HAVE_ICU
#include "provider/chinesecalendar.h"
#include "provider/hebrewcalendar.h"
#include "provider/indiancalendar.h"
#endif

using SubLabel = CalendarEvents::CalendarEventsPlugin::SubLabel;

class AlternateCalendarPluginPrivate
{
public:
    explicit AlternateCalendarPluginPrivate(AlternateCalendarPlugin *parent);
    ~AlternateCalendarPluginPrivate();

    void init();
    void loadEventsForDateRange(const QDate &startDate, const QDate &endDate);

private:
    std::unique_ptr<AbstractCalendarProvider> m_calendarProvider;

    // Cache lookup data
    QCache<QDate, SubLabel> m_subLabelsCache;

    // For updating config
    KConfigGroup m_generalConfigGroup;
    KConfigWatcher::Ptr m_configWatcher;

    CalendarSystem::System m_calendarSystem;
    int m_dateOffset; // For the (tabular) Islamic Civil calendar

    AlternateCalendarPlugin *q;
};

AlternateCalendarPluginPrivate::AlternateCalendarPluginPrivate(AlternateCalendarPlugin *parent)
    : q(parent)
{
    m_subLabelsCache.setMaxCost(42 * 3 /*previous, current, next*/);

    auto config = KSharedConfig::openConfig(QStringLiteral("plasma_calendar_alternatecalendar"));
    m_generalConfigGroup = config->group("General");
    m_configWatcher = KConfigWatcher::create(config);
    QObject::connect(m_configWatcher.get(), &KConfigWatcher::configChanged, q, &AlternateCalendarPlugin::updateSettings);
    init();
}

AlternateCalendarPluginPrivate::~AlternateCalendarPluginPrivate()
{
}

void AlternateCalendarPluginPrivate::init()
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
        m_calendarProvider.reset(new ChineseCalendarProvider(q, m_calendarSystem));
        break;
    case CalendarSystem::Indian:
        m_calendarProvider.reset(new IndianCalendarProvider(q, m_calendarSystem));
        break;
    case CalendarSystem::Hebrew:
        m_calendarProvider.reset(new HebrewCalendarProvider(q, m_calendarSystem));
        break;
#endif
#ifndef QT_BOOTSTRAPPED
    case CalendarSystem::Julian:
    case CalendarSystem::Milankovic:
#endif
#if QT_CONFIG(jalalicalendar)
    case CalendarSystem::Jalali:
#endif
#if QT_CONFIG(islamiccivilcalendar)
    case CalendarSystem::IslamicCivil:
#endif
        m_calendarProvider.reset(new QtCalendarProvider(q, m_calendarSystem));
        break;
    default:
        m_calendarProvider.reset(new AbstractCalendarProvider(q, m_calendarSystem));
    }

    // Clear the old cache when config is reloaded
    m_subLabelsCache.clear();
}

void AlternateCalendarPluginPrivate::loadEventsForDateRange(const QDate &startDate, const QDate &endDate)
{
    if (!endDate.isValid() || m_calendarSystem == CalendarSystem::Gregorian) {
        return;
    }

    QHash<QDate, QDate> alternateDatesData;
    QHash<QDate, CalendarEvents::CalendarEventsPlugin::SubLabel> subLabelsData;

    for (QDate date = startDate; date <= endDate && date.isValid(); date = date.addDays(1)) {
        const QDate offsetDate = date.addDays(m_dateOffset);
        const QCalendar::YearMonthDay alt = m_calendarProvider->fromGregorian(offsetDate);

        if (alt.day != date.day() || alt.month != date.month() || alt.year != date.year()) {
            alternateDatesData.insert(date, QDate(alt.year, alt.month, alt.day));
        }

        if (m_subLabelsCache.contains(date)) {
            subLabelsData.insert(date, *m_subLabelsCache.object(date));
        } else {
            const auto it = subLabelsData.insert(date, m_calendarProvider->subLabels(offsetDate));
            m_subLabelsCache.insert(date, new SubLabel(*it));
        }
    }

    if (alternateDatesData.size() > 0) {
        Q_EMIT q->alternateDateReady(alternateDatesData);
    }
    Q_EMIT q->subLabelReady(subLabelsData);
}

AlternateCalendarPlugin::AlternateCalendarPlugin(QObject *parent)
    : CalendarEvents::CalendarEventsPlugin(parent)
    , d(std::make_unique<AlternateCalendarPluginPrivate>(this))
{
}

AlternateCalendarPlugin::~AlternateCalendarPlugin()
{
}

void AlternateCalendarPlugin::loadEventsForDateRange(const QDate &startDate, const QDate &endDate)
{
    m_lastStartDate = startDate;
    m_lastEndDate = endDate;

    d->loadEventsForDateRange(startDate, endDate);
}

void AlternateCalendarPlugin::updateSettings()
{
    d->init();
    loadEventsForDateRange(m_lastStartDate, m_lastEndDate);
}
