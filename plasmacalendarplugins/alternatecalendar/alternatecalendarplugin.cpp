/*
    SPDX-FileCopyrightText: 2022 Fushan Wen <qydwhotmail@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "config-ICU.h"

#include "alternatecalendarplugin.h"

#include <QThreadPool>

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
    m_alternateDateCache.setMaxCost(42 * 6 /* (previous, current, next) * 2 */);
    m_sublabelCache.setMaxCost(42 * 6 /* (previous, current, next) * 2 */);

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

    std::vector<QDate> alternateDates;
    std::vector<QDate> sublabelDates;

    // Filter dates that are not in cache
    for (QDate date = startDate; date <= endDate && date.isValid(); date = date.addDays(1)) {
        if (!m_alternateDateCache.contains(date)) {
            alternateDates.push_back(date);
        }
        if (!m_sublabelCache.contains(date)) {
            sublabelDates.push_back(date);
        }
    }

    if (alternateDates.empty() && sublabelDates.empty()) {
        emitDataChangedSignal(startDate, endDate);
    } else {
        AbstractCalendarProvider *provider = nullptr;
        switch (m_calendarSystem) {
#if HAVE_ICU
        case CalendarSystem::Chinese:
            provider = new ChineseCalendarProvider(this, m_calendarSystem, std::move(alternateDates), std::move(sublabelDates));
            break;
        case CalendarSystem::Indian:
            provider = new IndianCalendarProvider(this, m_calendarSystem, std::move(alternateDates), std::move(sublabelDates));
            break;
        case CalendarSystem::Hebrew:
            provider = new HebrewCalendarProvider(this, m_calendarSystem, std::move(alternateDates), std::move(sublabelDates));
            break;
        case CalendarSystem::Jalali:
        case CalendarSystem::Islamic:
        case CalendarSystem::IslamicCivil:
        case CalendarSystem::IslamicUmalqura:
            provider = new IslamicCalendarProvider(this, m_calendarSystem, std::move(alternateDates), std::move(sublabelDates), m_dateOffset);
            break;
#else
        // Fall back to QtCalendar
        case CalendarSystem::Jalali:
        case CalendarSystem::IslamicCivil:
#endif
        case CalendarSystem::Julian:
        case CalendarSystem::Milankovic:
            provider = new QtCalendarProvider(this, m_calendarSystem, std::move(alternateDates), std::move(sublabelDates), m_dateOffset);
            break;
        default:
            provider = new AbstractCalendarProvider(this, m_calendarSystem, std::move(alternateDates), std::move(sublabelDates), m_dateOffset);
        }
        connect(provider,
                &AbstractCalendarProvider::dataReady,
                this,
                [this, startDate, endDate](const QHash<QDate, QCalendar::YearMonthDay> &alternateDatesData,
                                           const QHash<QDate, CalendarEvents::CalendarEventsPlugin::SubLabel> &sublabelData) {
                    // Fill cache
                    for (auto it = alternateDatesData.cbegin(); it != alternateDatesData.cend(); it = std::next(it)) {
                        m_alternateDateCache.insert(it.key(), new QCalendar::YearMonthDay(it.value()));
                    }
                    for (auto it = sublabelData.cbegin(); it != sublabelData.cend(); it = std::next(it)) {
                        m_sublabelCache.insert(it.key(), new SubLabel(it.value()));
                    }

                    emitDataChangedSignal(startDate, endDate);
                });
        QThreadPool::globalInstance()->start(provider);
    }
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

    // Clear the old cache when config is reloaded
    m_alternateDateCache.clear();
    m_sublabelCache.clear();
}

void AlternateCalendarPlugin::emitDataChangedSignal(const QDate &startDate, const QDate &endDate)
{
    QHash<QDate, QCalendar::YearMonthDay> fullAlternateDatesData;
    QHash<QDate, CalendarEvents::CalendarEventsPlugin::SubLabel> fullSublabelData;
    for (QDate date = startDate; date <= endDate && date.isValid(); date = date.addDays(1)) {
        if (m_alternateDateCache.contains(date)) {
            const QCalendar::YearMonthDay &alt = *m_alternateDateCache.object(date);
            if (alt.day != date.day() || alt.month != date.month() || alt.year != date.year()) {
                fullAlternateDatesData.insert(date, alt);
            }
        }
        if (m_sublabelCache.contains(date)) {
            fullSublabelData.insert(date, *m_sublabelCache.object(date));
        }
    }

    if (fullAlternateDatesData.size() > 0) {
        Q_EMIT alternateCalendarDateReady(fullAlternateDatesData);
    }
    Q_EMIT subLabelReady(fullSublabelData);
}

#include "moc_alternatecalendarplugin.cpp"