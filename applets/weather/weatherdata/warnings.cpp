/*
 * SPDX-FileCopyrightText: 2025 Bohdan Onofriichuk <bogdan.onofriuchuk@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "warnings.h"

Warnings::Warnings(QObject *parent)
    : QAbstractListModel(parent)
{
}

Warnings::~Warnings()
{
}

int Warnings::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid()) {
        return 0;
    }

    return m_warnings.count();
}

QVariant Warnings::data(const QModelIndex &index, int role) const
{
    if (index.row() >= m_warnings.count()) {
        return {};
    }

    const Warning &warning = m_warnings.at(index.row());

    switch (role) {
    case Priority:
        return warning.priority();
    case Description:
        return warning.description();
    case Info:
        return warning.info().has_value() ? *warning.info() : QVariant();
    case Timestamp:
        return warning.timestamp() ? *warning.timestamp() : QVariant();
    }

    return {};
}

QHash<int, QByteArray> Warnings::roleNames() const
{
    QHash<int, QByteArray> roles;

    roles[Description] = "description";
    roles[Info] = "info";
    roles[Priority] = "priority";
    roles[Timestamp] = "timestamp";

    return roles;
}

void Warnings::addWarning(const Warning &warning)
{
    m_warnings.append(warning);
    if (warning.priority() == High) {
        m_maxPriorityCount = m_maxPriorityCount + 1;
    }
}

void Warnings::addWarnings(const QList<Warning> &warnings)
{
    for (auto warning : warnings) {
        addWarning(warning);
    }
}

int Warnings::getMaxPriorityCount() const
{
    return m_maxPriorityCount;
}

int Warnings::getCount() const
{
    return m_warnings.count();
}

Warning::Warning(Warnings::PriorityClass priority, QString description)
    : m_priority(priority)
    , m_description(description)
{
}

Warning::~Warning()
{
}

Warnings::PriorityClass Warning::priority() const
{
    return m_priority;
}

QString Warning::description() const
{
    return m_description;
}

std::optional<QString> Warning::info() const
{
    return m_info;
}

std::optional<QString> Warning::timestamp() const
{
    return m_timestamp;
}

void Warning::setInfo(const QString &info)
{
    m_info = info;
}

void Warning::setTimestamp(const QString &timestamp)
{
    m_timestamp = timestamp;
}

#include "moc_warnings.cpp"
