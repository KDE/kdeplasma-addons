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
            if (m_nextDays.at(section).getMonthDay().has_value()) {
                return *m_nextDays.at(section).getMonthDay();
            }
            return {};
        }

        if (role == WeekDay) {
            if (m_nextDays.at(section).getWeekDay().has_value()) {
                return *m_nextDays.at(section).getWeekDay();
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
        const std::optional<FutureForecast> &forecast = m_nextDays.at(index.column()).getDaytime();

        if (!forecast.has_value()) {
            return {};
        }

        switch (role) {
        case ConditionIcon:
            return forecast->getConditionIcon().has_value() ? *forecast->getConditionIcon() : QVariant();
        case Condition:
            return forecast->getCondition().has_value() ? *forecast->getCondition() : QVariant();
        case HighTemp:
            return forecast->getHighTemp().has_value() ? *forecast->getHighTemp() : QVariant();
        case LowTemp:
            return forecast->getLowTemp().has_value() ? *forecast->getLowTemp() : QVariant();
        case ConditionProbability:
            return forecast->getConditionProbability().has_value() ? *forecast->getConditionProbability() : QVariant();
        }
    } else {
        const std::optional<FutureForecast> &forecast = m_nextDays.at(index.column()).getNight();

        if (!forecast.has_value()) {
            return {};
        }

        switch (role) {
        case ConditionIcon:
            return forecast->getConditionIcon().has_value() ? *forecast->getConditionIcon() : QVariant();
        case Condition:
            return forecast->getCondition().has_value() ? *forecast->getCondition() : QVariant();
        case HighTemp:
            return forecast->getHighTemp().has_value() ? *forecast->getHighTemp() : QVariant();
        case LowTemp:
            return forecast->getLowTemp().has_value() ? *forecast->getLowTemp() : QVariant();
        case ConditionProbability:
            return forecast->getConditionProbability().has_value() ? *forecast->getConditionProbability() : QVariant();
        }
    }

    return {};
}

void FutureDays::addDay(const FutureDayForecast &forecast)
{
    beginInsertColumns(QModelIndex(), m_nextDays.size(), m_nextDays.size());

    if (m_nextDays.isEmpty() && forecast.getDaytime().has_value()) {
        m_firstDayExist = true;
    }

    if (!m_isNightPresent && forecast.getNight()) {
        m_totalRows = 2;
        m_isNightPresent = true;
    }

    if ((forecast.getDaytime().has_value() && forecast.getDaytime()->getConditionProbability() != 0.0)
        || (forecast.getNight().has_value() && forecast.getNight()->getConditionProbability() != 0.0)) {
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

QString FutureDays::getFirstDayIcon() const
{
    const auto &forecast = m_nextDays.at(0).getDaytime();

    if (forecast.has_value()) {
        return *forecast->getConditionIcon();
    }
    return {};
}

bool FutureDays::getIsNightPresent() const
{
    return m_isNightPresent;
}

bool FutureDays::getHasProbability() const
{
    return m_hasProbability;
}

bool FutureDays::getFirstDayExist() const
{
    return m_firstDayExist;
}

int FutureDays::getDaysNumber() const
{
    return m_daysNumber;
}

FutureDayForecast::FutureDayForecast()
{
}

FutureDayForecast::~FutureDayForecast()
{
}

std::optional<int> FutureDayForecast::getMonthDay() const
{
    return m_monthDay;
}

std::optional<QString> FutureDayForecast::getWeekDay() const
{
    return m_weekDay;
}

std::optional<FutureForecast> FutureDayForecast::getDaytime() const
{
    return m_daytime;
}

std::optional<FutureForecast> FutureDayForecast::getNight() const
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

std::optional<QString> FutureForecast::getConditionIcon() const
{
    return m_conditionIcon;
}

std::optional<QString> FutureForecast::getCondition() const
{
    return m_condition;
}

std::optional<qreal> FutureForecast::getHighTemp() const
{
    return m_highTemp;
}

std::optional<qreal> FutureForecast::getLowTemp() const
{
    return m_lowTemp;
}

std::optional<qreal> FutureForecast::getConditionProbability() const
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
