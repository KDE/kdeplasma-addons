/*
 * SPDX-FileCopyrightText: 2025 Bohdan Onofriichuk <bogdan.onofriuchuk@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "futurehours.h"

#include <klocalizedstring.h>

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
    roles[Timestamp] = "timestamp";
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

    return m_allHours.count();
}

QVariant FutureHours::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation == Qt::Horizontal && m_allHours.count() > 1) {
        if (section >= m_allHours.count()) {
            return {};
        }

        if (role == Timestamp) {
            return m_allHours.at(section).timestamp();
        }
    }
    return {};
}

QVariant FutureHours::data(const QModelIndex &index, int role) const
{
    if (index.row() >= m_allHours.count()) {
        return {};
    }

    const std::optional<FutureHourForecast> &forecast = m_allHours.at(index.row());

    if (!forecast.has_value()) {
        return {};
    }

    switch (role) {
    case Timestamp:
        return forecast->timestamp();
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
    beginInsertRows(QModelIndex(), m_allHours.size(), m_allHours.size());

    if ((forecast.conditionProbability() != 0.0) || (forecast.conditionProbability() != 0.0)) {
        m_hasProbability = true;
    }

    m_allHours.append(forecast);
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
    const auto &forecast = m_allHours.at(0);

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
    return m_allHours.count();
}

FutureHourForecast::FutureHourForecast(const QDateTime &timestamp)
    : m_timestamp(timestamp)
{
}

FutureHourForecast::~FutureHourForecast()
{
}

QDateTime FutureHourForecast::timestamp() const
{
    return m_timestamp;
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
