/*
 * SPDX-FileCopyrightText: 2025 Bohdan Onofriichuk <bogdan.onofriuchuk@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#pragma once

#include <plasmaweatherdata_export.h>

#include <QAbstractListModel>
#include <QObjectBindableProperty>
#include <QString>

#include <qqmlintegration.h>

class Warning;

/*!
 * \class Warnings
 *
 * \brief List model with info about warnings
 *
 * "Description " + number, string, free text string, required
 * "Priority " + number, string, free text string, required
 * "Info " + number, string, free text string, optional
 * "Timestamp " + number, string, free text string, optional TODO: get standardized datetime
 */
class PLASMAWEATHERDATA_EXPORT Warnings : public QAbstractListModel
{
    Q_OBJECT

    Q_PROPERTY(int maxPriorityCount READ maxPriorityCount CONSTANT)
    Q_PROPERTY(int count READ count CONSTANT)

public:
    enum WarningModel {
        Description = Qt::UserRole + 1,
        Info,
        Priority,
        Timestamp,
    };

    Q_ENUM(WarningModel);

    enum PriorityClass {
        Low = 1,
        Medium,
        High,
        Extreme,
    };

    Q_ENUM(PriorityClass);

    explicit Warnings(QObject *parent = nullptr);
    ~Warnings() override;

    void addWarning(const Warning &warning);
    void addWarnings(const QList<Warning> &warnings);

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;

private:
    int maxPriorityCount() const;
    int count() const;

private:
    QList<Warning> m_warnings;

    int m_maxPriorityCount;
};

/*!
 * \class Warning
 *
 * \brief Data about one warning
 */
class PLASMAWEATHERDATA_EXPORT Warning
{
public:
    Warning(Warnings::PriorityClass priority, QString description);
    ~Warning();

    Warnings::PriorityClass priority() const;
    QString description() const;
    std::optional<QString> info() const;
    std::optional<QString> timestamp() const;

    void setInfo(const QString &info);
    void setTimestamp(const QString &timestamp);

private:
    Warnings::PriorityClass m_priority;
    QString m_description;
    std::optional<QString> m_info;
    std::optional<QString> m_timestamp;
};
