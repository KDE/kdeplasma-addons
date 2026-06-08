/*
 * SPDX-FileCopyrightText: 2025 Bohdan Onofriichuk <bogdan.onofriuchuk@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "futurehours.h"

#include <klocalizedstring.h>

FutureHoursPoints::FutureHoursPoints(const std::shared_ptr<FutureHours> &futureHours, QObject *parent)
    : QAbstractTableModel(parent)
    , m_minTemp(0)
    , m_maxTemp(0)
    , m_futureHours(futureHours)
{
    if (!m_futureHours->rowCount()) {
        return;
    }

    qreal minTemp = std::numeric_limits<qreal>::max();
    qreal maxTemp = std::numeric_limits<qreal>::min();
    for (int hourIndex = 0; hourIndex < m_futureHours->rowCount(); ++hourIndex) {
        QVariant minTempVariant = m_futureHours->data(m_futureHours->index(hourIndex), FutureHours::LowTemp);
        QVariant maxTempVariant = m_futureHours->data(m_futureHours->index(hourIndex), FutureHours::HighTemp);

        // Calculate min and max values according to what data ion provides
        if (minTempVariant.canConvert<qreal>() && maxTempVariant.canConvert<qreal>()) {
            minTemp = std::min(minTempVariant.toReal(), minTemp);
            maxTemp = std::max(maxTempVariant.toReal(), maxTemp);
        } else if (minTempVariant.canConvert<qreal>()) {
            minTemp = std::min(minTempVariant.toReal(), minTemp);
            maxTemp = std::max(minTempVariant.toReal(), maxTemp);
        } else if (maxTempVariant.canConvert<qreal>()) {
            minTemp = std::min(maxTempVariant.toReal(), minTemp);
            maxTemp = std::max(maxTempVariant.toReal(), maxTemp);
        }
    }

    m_maxTemp = maxTemp;
    m_minTemp = minTemp;
}

FutureHoursPoints::~FutureHoursPoints()
{
}

int FutureHoursPoints::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid()) {
        return 0;
    }

    return EndRow;
}

int FutureHoursPoints::columnCount(const QModelIndex &parent) const
{
    if (parent.isValid()) {
        return 0;
    }

    return m_futureHours->rowCount();
}

QVariant FutureHoursPoints::data(const QModelIndex &index, int role) const
{
    if (!index.isValid()) {
        return {};
    }

    if (role != Qt::DisplayRole) {
        return {};
    }

    if (index.row() == Timestamp) {
        return m_futureHours->data(m_futureHours->index(index.column()), FutureHours::Timestamp).toDateTime();
    } else if (index.row() == Temperature) {
        QVariant highTemp = m_futureHours->data(m_futureHours->index(index.column()), FutureHours::HighTemp);
        QVariant lowTemp = m_futureHours->data(m_futureHours->index(index.column()), FutureHours::LowTemp);
        // If high and low temperature present then return middle value. Otherwise
        // return the value which is present;
        if (highTemp.isValid() && lowTemp.isValid()) {
            return std::midpoint(highTemp.toReal(), lowTemp.toReal());
        } else if (highTemp.isValid()) {
            return highTemp;
        } else if (lowTemp.isValid()) {
            return lowTemp;
        }
        return {};
    }

    return {};
}

int FutureHoursPoints::pointsNumber() const
{
    return m_futureHours->rowCount();
}

QDateTime FutureHoursPoints::minDate() const
{
    QVariant date = m_futureHours->data(m_futureHours->index(0), FutureHours::Timestamp);
    if (date.canConvert<QDateTime>()) {
        return date.toDateTime();
    }
    return {};
}

QDateTime FutureHoursPoints::maxDate() const
{
    QVariant date = m_futureHours->data(m_futureHours->index(m_futureHours->rowCount() - 1), FutureHours::Timestamp);
    if (date.canConvert<QDateTime>()) {
        return date.toDateTime();
    }
    return {};
}

qreal FutureHoursPoints::minTemp() const
{
    return m_minTemp;
}

qreal FutureHoursPoints::maxTemp() const
{
    return m_maxTemp;
}

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
