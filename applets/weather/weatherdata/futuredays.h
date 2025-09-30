/*
 * SPDX-FileCopyrightText: 2025 Bohdan Onofriichuk <bogdan.onofriuchuk@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#pragma once

#include <plasmaweatherdata_export.h>

#include <QAbstractTableModel>
#include <QObjectBindableProperty>
#include <QString>

#include <qqmlintegration.h>

/*!
 * \class FutureForecast
 *
 * \brief Data about part of the day
 */
class PLASMAWEATHERDATA_EXPORT FutureForecast
{
public:
    explicit FutureForecast();
    ~FutureForecast();

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
    std::optional<QString> m_conditionIcon;
    std::optional<QString> m_condition;
    std::optional<qreal> m_highTemp;
    std::optional<qreal> m_lowTemp;
    std::optional<qreal> m_conditionProbability;
};

/*!
 * \class FutureDayForecast
 *
 * \brief Data about full day
 */
class PLASMAWEATHERDATA_EXPORT FutureDayForecast
{
public:
    explicit FutureDayForecast();
    ~FutureDayForecast();

    std::optional<int> monthDay() const;
    std::optional<QString> weekDay() const;

    std::optional<FutureForecast> daytime() const;
    std::optional<FutureForecast> night() const;

    void setMonthDay(int monthDay);
    void setWeekDay(const QString &weekDay);

    void setDaytime(const FutureForecast &daytime);
    void setNight(const FutureForecast &night);

private:
    std::optional<int> m_monthDay;
    std::optional<QString> m_weekDay;
    std::optional<FutureForecast> m_daytime;
    std::optional<FutureForecast> m_night;
};

/*!
 * \class FutureDays
 *
 * \brief Data about next days
 *
 * monthDay: day of month, optional,
 * weekDay: day of week, optional,
 * period: time of day, required,
 * conditionIcon: xdg icon name for current weather observation, optional,
 * condition: free text string for weather condition, optional
 * highTemp: number of highest temperature (using general unit), optional
 * lowTemp: number of lowest temperature (using general unit), optional
 * conditionProbability: chance of conditions to happen, optional
 */
class PLASMAWEATHERDATA_EXPORT FutureDays : public QAbstractTableModel
{
    Q_OBJECT
    QML_ANONYMOUS

    Q_PROPERTY(QString firstDayIcon READ firstDayIcon CONSTANT)
    Q_PROPERTY(bool isNightPresent READ isNightPresent CONSTANT)
    Q_PROPERTY(bool hasProbability READ hasProbability CONSTANT)
    Q_PROPERTY(bool firstDayExist READ firstDayExist CONSTANT)
    Q_PROPERTY(int daysNumber READ daysNumber CONSTANT)

public:
    enum NextDaysModels {
        MonthDay = Qt::UserRole + 1,
        WeekDay,
        Period,
        ConditionIcon,
        Condition,
        HighTemp,
        LowTemp,
        ConditionProbability,
    };

    Q_ENUM(NextDaysModels)

    explicit FutureDays(QObject *parent = nullptr);
    ~FutureDays() override;

    void addDay(const FutureDayForecast &forecast);

    void addDays(const QList<FutureDayForecast> &forecasts);

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    QHash<int, QByteArray> roleNames() const override;

private:
    QString firstDayIcon() const;
    bool isNightPresent() const;
    bool hasProbability() const;
    bool firstDayExist() const;
    int daysNumber() const;

private:
    enum Columns {
        Day = 0,
        Night,
    };

    QList<FutureDayForecast> m_nextDays;

    bool m_isNightPresent;
    bool m_hasProbability;
    bool m_firstDayExist;
    int m_daysNumber;

    int m_totalRows;
};
