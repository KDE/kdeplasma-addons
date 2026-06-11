/*
 * SPDX-FileCopyrightText: 2025 Bohdan Onofriichuk <bogdan.onofriuchuk@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "futuredays.h"

#include <klocalizedstring.h>

FutureDaysPoints::FutureDaysPoints(const std::shared_ptr<FutureDays> &futureDays, QObject *parent)
    : QAbstractTableModel(parent)
    , m_minTemp(0)
    , m_maxTemp(0)
    , m_futureDays(futureDays)
{
    if (!m_futureDays->rowCount()) {
        return;
    }

    qreal minTemp = std::numeric_limits<qreal>::max();
    qreal maxTemp = std::numeric_limits<qreal>::min();
    for (int dayTimeIndex = 0; dayTimeIndex < m_futureDays->rowCount(); ++dayTimeIndex) {
        for (int dayIndex = 0; dayIndex < m_futureDays->columnCount(); ++dayIndex) {
            QVariant minTempVariant = m_futureDays->data(m_futureDays->index(dayTimeIndex, dayIndex), FutureDays::LowTemp);
            QVariant maxTempVariant = m_futureDays->data(m_futureDays->index(dayTimeIndex, dayIndex), FutureDays::HighTemp);

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
    }

    m_maxTemp = maxTemp;
    m_minTemp = minTemp;
}

FutureDaysPoints::~FutureDaysPoints()
{
}

int FutureDaysPoints::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid()) {
        return 0;
    }

    return EndRow;
}

int FutureDaysPoints::columnCount(const QModelIndex &parent) const
{
    if (parent.isValid()) {
        return 0;
    }

    return m_futureDays->columnCount();
}

QVariant FutureDaysPoints::data(const QModelIndex &index, int role) const
{
    if (!index.isValid()) {
        return {};
    }

    if (role != Qt::DisplayRole) {
        return {};
    }

    if (index.row() == Timestamp) {
        for (int rowPosition = 0; rowPosition < m_futureDays->rowCount(); ++rowPosition) {
            // reset time to start from the begining of the day. Otherwise graphs that depends on
            // this can change the position of points which will lead to broken alignment
            return QDateTime(m_futureDays->data(m_futureDays->index(rowPosition, index.column()), FutureDays::Timestamp).toDate(), QTime());
        }
    } else if (index.row() == Temperature) {
        // Some forecasts contains day and night forecast. So calculate the mid value using day and night.
        std::optional<qreal> averageOverallTemp = std::nullopt;
        for (int rowPosition = 0; rowPosition < m_futureDays->rowCount(); ++rowPosition) {
            std::optional<qreal> averageTemp = std::nullopt;
            QVariant highTemp = m_futureDays->data(m_futureDays->index(rowPosition, index.column()), FutureDays::HighTemp);
            QVariant lowTemp = m_futureDays->data(m_futureDays->index(rowPosition, index.column()), FutureDays::LowTemp);

            // If high and low temperature present then return middle value. Otherwise
            // return the value which is present;
            if (highTemp.isValid() && lowTemp.isValid()) {
                averageTemp = std::midpoint(highTemp.toReal(), lowTemp.toReal());
            } else if (highTemp.isValid()) {
                averageTemp = highTemp.toReal();
            } else if (lowTemp.isValid()) {
                averageTemp = lowTemp.toReal();
            }

            if (!averageOverallTemp.has_value() && averageTemp.has_value()) {
                averageOverallTemp = averageTemp;
            } else if (averageOverallTemp.has_value() && averageTemp.has_value()) {
                averageOverallTemp = std::midpoint(averageTemp.value(), averageOverallTemp.value());
            }
        }
        if (averageOverallTemp.has_value()) {
            return averageOverallTemp.value();
        }
    }

    return {};
}

qreal FutureDaysPoints::minTemp() const
{
    return m_minTemp;
}

qreal FutureDaysPoints::maxTemp() const
{
    return m_maxTemp;
}

QDateTime FutureDaysPoints::minDate() const
{
    // It is possible that the day forecast is not present. So check for the night forecast too
    for (int rowPosition = 0; rowPosition < m_futureDays->rowCount(); ++rowPosition) {
        QVariant date = m_futureDays->data(m_futureDays->index(rowPosition, 0), FutureDays::Timestamp);
        if (date.canConvert<QDateTime>()) {
            // reset time to start from the begining of the day. Otherwise graphs that depends on
            // this can change the position of points which will lead to broken alignment
            return QDateTime(date.toDate(), QTime());
        }
    }
    return {};
}

QDateTime FutureDaysPoints::maxDate() const
{
    // It is possible that the day forecast is not present. So check for the night forecast too
    for (int rowPosition = 0; rowPosition < m_futureDays->rowCount(); ++rowPosition) {
        QVariant date = m_futureDays->data(m_futureDays->index(rowPosition, m_futureDays->columnCount() - 1), FutureDays::Timestamp);
        if (date.canConvert<QDateTime>()) {
            // reset time to start from the begining of the day. Otherwise graphs that depends on
            // this can change the position of points which will lead to broken alignment
            return QDateTime(date.toDate(), QTime());
        }
    }
    return {};
}

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
    roles[Timestamp] = "timestamp";
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

        if (role == Timestamp) {
            return m_nextDays.at(section).timestamp();
        }
    }

    if (orientation == Qt::Vertical && m_totalRows > 1) {
        if (role == Period) {
            if (section == Day) {
                return i18nc("Short for Day", "D");
            }
            if (m_isNightPresent && section == Night) {
                return i18nc("Short for Night", "N");
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

    if (role == Timestamp) {
        return m_nextDays.at(index.column()).timestamp();
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
    for (const auto &forecast : forecasts) {
        addDay(forecast);
    }
}

QString FutureDays::firstDayIcon() const
{
    const auto &dayForecast = m_nextDays.at(0);

    // Check first for icon from the datetime forecast
    if (auto daytime = dayForecast.daytime(); daytime.has_value()) {
        if (auto conditionIcon = daytime->conditionIcon(); conditionIcon.has_value()) {
            return *conditionIcon;
        }
        return {};
    }

    // If the datetime forecast is not present try the night forecast
    if (auto night = dayForecast.night(); night.has_value()) {
        if (auto conditionIcon = night->conditionIcon(); conditionIcon.has_value()) {
            return *conditionIcon;
        }
        return {};
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

FutureDayForecast::FutureDayForecast(const QDateTime &timestamp)
    : m_timestamp(timestamp)
{
}

FutureDayForecast::~FutureDayForecast()
{
}

QDateTime FutureDayForecast::timestamp() const
{
    return m_timestamp;
}

std::optional<FutureForecast> FutureDayForecast::daytime() const
{
    return m_daytime;
}

std::optional<FutureForecast> FutureDayForecast::night() const
{
    return m_night;
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
