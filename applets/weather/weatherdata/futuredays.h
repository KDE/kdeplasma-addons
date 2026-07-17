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
    std::optional<qreal> generalTemp() const;
    std::optional<qreal> conditionProbability() const;

    void setConditionIcon(const QString &conditionIcon);
    void setCondition(const QString &condition);
    void setHighTemp(qreal highTemp);
    void setLowTemp(qreal lowTemp);
    void setGeneralTemp(qreal generalTemp);
    void setConditionProbability(qreal conditionProbability);

private:
    std::optional<QString> m_conditionIcon;
    std::optional<QString> m_condition;
    std::optional<qreal> m_highTemp;
    std::optional<qreal> m_lowTemp;
    std::optional<qreal> m_generalTemp;
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
    explicit FutureDayForecast(const QDateTime &timestamp);
    ~FutureDayForecast();

    QDateTime timestamp() const;

    std::optional<FutureForecast> daytime() const;
    std::optional<FutureForecast> night() const;

    void setDaytime(const FutureForecast &daytime);
    void setNight(const FutureForecast &night);

private:
    QDateTime m_timestamp;
    std::optional<FutureForecast> m_daytime;
    std::optional<FutureForecast> m_night;
};

/*!
 * \class FutureDays
 *
 * \brief Data about next days
 *
 * Timestamp: date of the forecast, optional,
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
        Timestamp = Qt::UserRole + 1,
        Period,
        ConditionIcon,
        Condition,
        HighTemp,
        LowTemp,
        GeneralTemp,
        ConditionProbability,
    };

    Q_ENUM(NextDaysModels)

    enum Columns {
        Day = 0,
        Night,
    };

    Q_ENUM(Columns)

    explicit FutureDays(QObject *parent = nullptr);
    ~FutureDays() override;

    void addDay(const FutureDayForecast &forecast);

    void addDays(const QList<FutureDayForecast> &forecasts);

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    QHash<int, QByteArray> roleNames() const override;

    bool isNightPresent() const;
    bool hasProbability() const;

private:
    QString firstDayIcon() const;
    bool firstDayExist() const;
    int daysNumber() const;

private:
    QList<FutureDayForecast> m_nextDays;

    bool m_isNightPresent;
    bool m_hasProbability;
    bool m_firstDayExist;
    int m_daysNumber;

    int m_totalRows;
};

struct FutureDaysForeign {
    Q_GADGET
    QML_FOREIGN(FutureDays)
    QML_NAMED_ELEMENT(FutureDays)
    QML_UNCREATABLE("Enums only")
};

class PLASMAWEATHERDATA_EXPORT FutureDaysPoints : public QAbstractTableModel
{
    Q_OBJECT
    QML_ANONYMOUS

    Q_PROPERTY(bool highLowTempPresent READ highLowTempPresent CONSTANT)
    Q_PROPERTY(bool hasProbability READ hasProbability CONSTANT)
    Q_PROPERTY(QDateTime minDate READ minDate CONSTANT)
    Q_PROPERTY(QDateTime maxDate READ maxDate CONSTANT)
    Q_PROPERTY(qreal minTemp READ minTemp CONSTANT)
    Q_PROPERTY(qreal maxTemp READ maxTemp CONSTANT)

public:
    enum RowsData {
        Timestamp = 0,
        GeneralTemp,
        HighTemp,
        LowTemp,
        ConditionProbability,
        EndRow,
    };

    Q_ENUM(RowsData)

    explicit FutureDaysPoints(const std::shared_ptr<FutureDays> &futureDays, QObject *parent = nullptr);
    ~FutureDaysPoints() override;

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;

    QVariant data(const QModelIndex &index, int role) const override;

    Q_INVOKABLE QVariant displayTemperature(int dayIndex, RowsData row) const;
    Q_INVOKABLE QVariant displayConditionProbability(int dayIndex) const;

private:
    QVariant aggregatedValue(int dayIndex, RowsData row) const;

    bool hasProbability() const;
    bool highLowTempPresent() const;
    QDateTime minDate() const;
    QDateTime maxDate() const;
    qreal minTemp() const;
    qreal maxTemp() const;

private:
    bool m_highLowTempPresent;

    qreal m_minTemp;
    qreal m_maxTemp;

    std::shared_ptr<FutureDays> m_futureDays;
};

struct FutureDaysPointsForeign {
    Q_GADGET
    QML_FOREIGN(FutureDaysPoints)
    QML_NAMED_ELEMENT(FutureDaysPoints)
    QML_UNCREATABLE("Enums only")
};
