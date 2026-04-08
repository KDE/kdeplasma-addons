/*
 * SPDX-FileCopyrightText: 2025 Bohdan Onofriichuk <bogdan.onofriuchuk@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "futurehours.h"

#include <klocalizedstring.h>

// Limit maximum hour forecasts to day
static const uint MAX_FORECASTS = 24;

FutureHours::FutureHours(QObject *parent)
    : QAbstractListModel(parent)
    , m_hasProbability(false)
{
}

FutureHours::~FutureHours()
{
}

QHash<int, QByteArray> FutureHours::roleNames() const
{
    QHash<int, QByteArray> roles;
    roles[Time] = "time";
    roles[ConditionIcon] = "conditionIcon";
    roles[Condition] = "condition";
    roles[HighTemp] = "highTemp";
    roles[LowTemp] = "lowTemp";
    roles[ConditionProbability] = "conditionProbability";
    return roles;
}

int FutureHours::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid()) {
        return 0;
    }

    return m_nextHours.count();
}

QVariant FutureHours::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation == Qt::Horizontal && m_nextHours.count() > 1) {
        if (section >= m_nextHours.count()) {
            return {};
        }

        if (role == Time) {
            return m_nextHours.at(section).time();
        }
    }
    return {};
}

QVariant FutureHours::data(const QModelIndex &index, int role) const
{
    if (index.row() >= m_nextHours.count()) {
        return {};
    }

    const std::optional<FutureHourForecast> &forecast = m_nextHours.at(index.row());

    if (!forecast.has_value()) {
        return {};
    }

    switch (role) {
    case Time:
        return forecast->time();
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

    return {};
}

void FutureHours::addHour(const FutureHourForecast &forecast)
{
    beginInsertRows(QModelIndex(), m_nextHours.size(), m_nextHours.size());

    if (m_nextHours.count() > MAX_FORECASTS) {
        return;
    }

    if ((forecast.conditionProbability() != 0.0) || (forecast.conditionProbability() != 0.0)) {
        m_hasProbability = true;
    }

    m_nextHours.append(forecast);
    endInsertRows();
}

void FutureHours::addHours(const QList<FutureHourForecast> &forecasts)
{
    beginResetModel();
    for (const auto &forecast : forecasts) {
        addHour(forecast);
    }
    endResetModel();
}

QString FutureHours::firstDayIcon() const
{
    const auto &forecast = m_nextHours.at(0);

    // Check first for icon from the datetime forecast
    if (auto conditionIcon = forecast.conditionIcon(); conditionIcon.has_value()) {
        return *conditionIcon;
    }
    return {};
}

bool FutureHours::hasProbability() const
{
    return m_hasProbability;
}

int FutureHours::hoursNumber() const
{
    return m_nextHours.count();
}

FutureHourForecast::FutureHourForecast(const QTime &time)
    : m_time(time)
{
}

FutureHourForecast::~FutureHourForecast()
{
}

QTime FutureHourForecast::time() const
{
    return m_time;
}

std::optional<QString> FutureHourForecast::conditionIcon() const
{
    return m_conditionIcon;
}

std::optional<QString> FutureHourForecast::condition() const
{
    return m_condition;
}

std::optional<qreal> FutureHourForecast::highTemp() const
{
    return m_highTemp;
}

std::optional<qreal> FutureHourForecast::lowTemp() const
{
    return m_lowTemp;
}

std::optional<qreal> FutureHourForecast::conditionProbability() const
{
    return m_conditionProbability;
}

void FutureHourForecast::setConditionIcon(const QString &conditionIcon)
{
    m_conditionIcon = conditionIcon;
}

void FutureHourForecast::setCondition(const QString &condition)
{
    m_condition = condition;
}

void FutureHourForecast::setHighTemp(qreal highTemp)
{
    m_highTemp = highTemp;
}

void FutureHourForecast::setLowTemp(qreal lowTemp)
{
    m_lowTemp = lowTemp;
}

void FutureHourForecast::setConditionProbability(qreal conditionProbability)
{
    m_conditionProbability = conditionProbability;
}

#include "moc_futurehours.cpp"
