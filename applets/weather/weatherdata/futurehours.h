/*
 * SPDX-FileCopyrightText: 2025 Bohdan Onofriichuk <bogdan.onofriuchuk@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#pragma once

#include <plasmaweatherdata_export.h>

#include <QAbstractTableModel>
#include <QDateTime>
#include <QObjectBindableProperty>
#include <QString>

#include <qqmlintegration.h>

/*!
 * \class FutureHourForecast
 *
 * \brief Data about hour of the day
 */
class PLASMAWEATHERDATA_EXPORT FutureHourForecast
{
public:
    explicit FutureHourForecast(const QDateTime &timestamp);
    ~FutureHourForecast();

    QDateTime timestamp() const;

    std::optional<QString> conditionIcon() const;
    std::optional<QString> condition() const;
    std::optional<qreal> highTemp() const;
    std::optional<qreal> lowTemp() const;
    std::optional<qreal> conditionProbability() const;

    void setConditionIcon(const QString &conditionIcon);
    void setCondition(const QString &condition);
    void setHighTemp(qreal highTemp);
    void setLowTemp(qreal lowTemp);
    void setConditionProbability(qreal conditionProbability);

private:
    QDateTime m_timestamp;
    std::optional<QString> m_conditionIcon;
    std::optional<QString> m_condition;
    std::optional<qreal> m_highTemp;
    std::optional<qreal> m_lowTemp;
    std::optional<qreal> m_conditionProbability;
};

/*!
 * \class FutureHours
 *
 * \brief Data about next hours
 *
 * hour: hour of day, required,
 * conditionIcon: xdg icon name for current weather observation, optional,
 * condition: free text string for weather condition, optional
 * highTemp: number of highest temperature (using general unit), optional
 * lowTemp: number of lowest temperature (using general unit), optional
 * conditionProbability: chance of conditions to happen, optional
 */
class PLASMAWEATHERDATA_EXPORT FutureHours : public QAbstractListModel
{
    Q_OBJECT
    QML_ANONYMOUS

    Q_PROPERTY(bool hasProbability READ hasProbability CONSTANT)
    Q_PROPERTY(int hoursNumber READ hoursNumber CONSTANT)

public:
    enum NextHoursModels {
        Timestamp = Qt::UserRole + 1,
        ConditionIcon,
        Condition,
        HighTemp,
        LowTemp,
        ConditionProbability,
    };

    Q_ENUM(NextHoursModels)

    explicit FutureHours(QObject *parent = nullptr);
    ~FutureHours() override;

    void addHour(const FutureHourForecast &forecast);

    void addHours(const QList<FutureHourForecast> &forecasts);

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    QHash<int, QByteArray> roleNames() const override;

private:
    QString firstDayIcon() const;
    bool isNightPresent() const;
    bool hasProbability() const;
    bool firstDayExist() const;
    int hoursNumber() const;

private:
    QList<FutureHourForecast> m_allHours;

    bool m_hasProbability;
};
