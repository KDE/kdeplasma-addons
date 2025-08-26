/*
 * SPDX-FileCopyrightText: 2025 Bohdan Onofriichuk <bogdan.onofriuchuk@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "futuredays.h"

#include <klocalizedstring.h>

FutureDays::FutureDays(QObject *parent)
    : QAbstractTableModel(parent)
    , m_isNightPresent(false)
    , m_hasProbability(false)
    , m_firstDayExist(false)
    , m_daysNumber(0)
    , m_totalRows(1) // only day forecasts without night forecasts
{
}

FutureDays::~FutureDays()
{
}

QHash<int, QByteArray> FutureDays::roleNames() const
{
    QHash<int, QByteArray> roles;
    roles[MonthDay] = "monthDay";
    roles[WeekDay] = "weekDay";
    roles[Period] = "period";
    roles[ConditionIcon] = "conditionIcon";
    roles[Condition] = "condition";
    roles[HighTemp] = "highTemp";
    roles[LowTemp] = "lowTemp";
    roles[ConditionProbability] = "conditionProbability";
    return roles;
}

int FutureDays::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid()) {
        return 0;
    }

    return m_totalRows;
}

int FutureDays::columnCount(const QModelIndex &parent) const
{
    if (parent.isValid()) {
        return 0;
    }

    return m_nextDays.count();
}

QVariant FutureDays::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation == Qt::Horizontal && m_nextDays.count() > 1) {
        if (section >= m_nextDays.count()) {
            return {};
        }

        if (role == MonthDay) {
            if (m_nextDays.at(section).monthDay().has_value()) {
                return *m_nextDays.at(section).monthDay();
            }
            return {};
        }

        if (role == WeekDay) {
            if (m_nextDays.at(section).weekDay().has_value()) {
                return *m_nextDays.at(section).weekDay();
            }
            return {};
        }
    }

    if (orientation == Qt::Vertical && m_totalRows > 1) {
        if (role == Period) {
            if (section == Day) {
                return i18n("Day");
            }
            if (m_isNightPresent && section == Night) {
                return i18n("Night");
            }
        }
    }
    return {};
}

QVariant FutureDays::data(const QModelIndex &index, int role) const
{
    if (index.column() >= m_nextDays.count()) {
        return {};
    }

    if (index.row() >= m_totalRows) {
        return {};
    }

    if (index.row() == 0) {
        const std::optional<FutureForecast> &forecast = m_nextDays.at(index.column()).daytime();

        if (!forecast.has_value()) {
            return {};
        }

        switch (role) {
        case ConditionIcon:
            return forecast->conditionIcon().has_value() ? *forecast->conditionIcon() : QVariant();
        case Condition:
            return forecast->condition().has_value() ? *forecast->condition() : QVariant();
        case HighTemp:
            return forecast->highTemp().has_value() ? *forecast->highTemp() : QVariant();
        case LowTemp:
            return forecast->lowTemp().has_value() ? *forecast->lowTemp() : QVariant();
        case ConditionProbability:
            return forecast->conditionProbability().has_value() ? *forecast->conditionProbability() : QVariant();
        }
    } else {
        const std::optional<FutureForecast> &forecast = m_nextDays.at(index.column()).night();

        if (!forecast.has_value()) {
            return {};
        }

        switch (role) {
        case ConditionIcon:
            return forecast->conditionIcon().has_value() ? *forecast->conditionIcon() : QVariant();
        case Condition:
            return forecast->condition().has_value() ? *forecast->condition() : QVariant();
        case HighTemp:
            return forecast->highTemp().has_value() ? *forecast->highTemp() : QVariant();
        case LowTemp:
            return forecast->lowTemp().has_value() ? *forecast->lowTemp() : QVariant();
        case ConditionProbability:
            return forecast->conditionProbability().has_value() ? *forecast->conditionProbability() : QVariant();
        }
    }

    return {};
}

void FutureDays::addDay(const FutureDayForecast &forecast)
{
    beginInsertColumns(QModelIndex(), m_nextDays.size(), m_nextDays.size());

    if (m_nextDays.isEmpty() && forecast.daytime().has_value()) {
        m_firstDayExist = true;
    }

    if (!m_isNightPresent && forecast.night()) {
        m_totalRows = 2;
        m_isNightPresent = true;
    }

    if ((forecast.daytime().has_value() && forecast.daytime()->conditionProbability() != 0.0)
        || (forecast.night().has_value() && forecast.night()->conditionProbability() != 0.0)) {
        m_hasProbability = true;
    }

    m_nextDays.append(forecast);
    endInsertColumns();
    m_daysNumber = m_nextDays.count();
}

void FutureDays::addDays(const QList<FutureDayForecast> &forecasts)
{
    beginResetModel();
    for (const auto &forecast : forecasts) {
        addDay(forecast);
    }
    endResetModel();
}

QString FutureDays::firstDayIcon() const
{
    const auto &forecast = m_nextDays.at(0).daytime();

    if (forecast.has_value()) {
        return *forecast->conditionIcon();
    }
    return {};
}

bool FutureDays::isNightPresent() const
{
    return m_isNightPresent;
}

bool FutureDays::hasProbability() const
{
    return m_hasProbability;
}

bool FutureDays::firstDayExist() const
{
    return m_firstDayExist;
}

int FutureDays::daysNumber() const
{
    return m_daysNumber;
}

FutureDayForecast::FutureDayForecast()
{
}

FutureDayForecast::~FutureDayForecast()
{
}

std::optional<int> FutureDayForecast::monthDay() const
{
    return m_monthDay;
}

std::optional<QString> FutureDayForecast::weekDay() const
{
    return m_weekDay;
}

std::optional<FutureForecast> FutureDayForecast::daytime() const
{
    return m_daytime;
}

std::optional<FutureForecast> FutureDayForecast::night() const
{
    return m_night;
}

void FutureDayForecast::setMonthDay(int monthDay)
{
    m_monthDay = monthDay;
}

void FutureDayForecast::setWeekDay(const QString &weekDay)
{
    m_weekDay = weekDay;
}

void FutureDayForecast::setDaytime(const FutureForecast &daytime)
{
    m_daytime = daytime;
}

void FutureDayForecast::setNight(const FutureForecast &night)
{
    m_night = night;
}

FutureForecast::FutureForecast()
{
}

FutureForecast::~FutureForecast()
{
}

std::optional<QString> FutureForecast::conditionIcon() const
{
    return m_conditionIcon;
}

std::optional<QString> FutureForecast::condition() const
{
    return m_condition;
}

std::optional<qreal> FutureForecast::highTemp() const
{
    return m_highTemp;
}

std::optional<qreal> FutureForecast::lowTemp() const
{
    return m_lowTemp;
}

std::optional<qreal> FutureForecast::conditionProbability() const
{
    return m_conditionProbability;
}

void FutureForecast::setConditionIcon(const QString &conditionIcon)
{
    m_conditionIcon = conditionIcon;
}

void FutureForecast::setCondition(const QString &condition)
{
    m_condition = condition;
}

void FutureForecast::setHighTemp(qreal highTemp)
{
    m_highTemp = highTemp;
}

void FutureForecast::setLowTemp(qreal lowTemp)
{
    m_lowTemp = lowTemp;
}

void FutureForecast::setConditionProbability(qreal conditionProbability)
{
    m_conditionProbability = conditionProbability;
}

#include "moc_futuredays.cpp"
