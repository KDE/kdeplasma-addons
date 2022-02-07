/*
    SPDX-FileCopyrightText: 2022 Fushan Wen <qydwhotmail@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "alternatecalendarplugin.h"

#include <KConfigGroup>
#include <KConfigWatcher>
#include <KSharedConfig>

#include "provider/qtcalendar.h"

class AlternateCalendarPluginPrivate
{
public:
    explicit AlternateCalendarPluginPrivate(AlternateCalendarPlugin *parent);
    ~AlternateCalendarPluginPrivate();

    void init();
    AbstractCalendarProvider *calendarProvider() const;

    CalendarSystem::System m_calendarSystem;
    int m_dateOffset; // For the (tabular) Islamic Civil calendar

private:
    std::unique_ptr<AbstractCalendarProvider> m_calendarProvider;

    // For updating config
    KConfigGroup m_generalConfigGroup;
    KConfigWatcher::Ptr m_configWatcher;

    AlternateCalendarPlugin *q;
};

AlternateCalendarPluginPrivate::AlternateCalendarPluginPrivate(AlternateCalendarPlugin *parent)
    : q(parent)
{
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
    m_calendarSystem = static_cast<CalendarSystem::System>(m_generalConfigGroup.readEntry("calendarSystem", static_cast<int>(CalendarSystem::Gregorian)));
    m_dateOffset = m_generalConfigGroup.readEntry("dateOffset", 0);

    // Load/Reload the calendar provider
    switch (m_calendarSystem) {
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
}

AbstractCalendarProvider *AlternateCalendarPluginPrivate::calendarProvider() const
{
    return m_calendarProvider.get();
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
    if (!endDate.isValid() || d->m_calendarSystem == CalendarSystem::Gregorian) {
        return;
    }

    m_lastStartDate = startDate;
    m_lastEndDate = endDate;

    QHash<QDate, QDate> alternateDatesData;
    QHash<QDate, SubLabel> subLabelsData;

    const int dateOffset = d->m_dateOffset;

    for (QDate date = startDate; date <= endDate && date.isValid(); date = date.addDays(1)) {
        const QDate offsetDate = date.addDays(dateOffset);
        if (const QDate alt = d->calendarProvider()->fromGregorian(offsetDate); alt != date) {
            alternateDatesData.insert(date, alt);
        }
        subLabelsData.insert(date, d->calendarProvider()->subLabels(offsetDate));
    }

    if (alternateDatesData.size() > 0) {
        Q_EMIT alternateDateReady(alternateDatesData);
    }
    Q_EMIT subLabelReady(subLabelsData);
}

void AlternateCalendarPlugin::updateSettings()
{
    d->init();
    loadEventsForDateRange(m_lastStartDate, m_lastEndDate);
}