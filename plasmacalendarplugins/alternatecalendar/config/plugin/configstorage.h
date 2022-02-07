/*
    SPDX-FileCopyrightText: 2018 Friedrich W. H. Kossebau <kossebau@kde.org>
    SPDX-FileCopyrightText: 2022 Fushan Wen <qydwhotmail@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef CONFIGSTORAGE_H
#define CONFIGSTORAGE_H

#include <QAbstractListModel>

#include <KConfigGroup>

#include "../../calendarsystem.h"

class CalendarSystemModel : public QAbstractListModel
{
    Q_OBJECT

public:
    enum Role {
        ValueRole = Qt::UserRole,
    };

    explicit CalendarSystemModel(QObject *parent = nullptr);

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QHash<int, QByteArray> roleNames() const override;

    int indexOf(CalendarSystem::System value) const;

private:
    struct CalendarSystemItem {
        QString text;
        CalendarSystem::System value;
    };

    std::vector<CalendarSystemItem> m_items;
};

class ConfigStorage : public QObject
{
    Q_OBJECT

    /**
     * The current choosen calendar system
     */
    Q_PROPERTY(CalendarSystem::System calendarSystem MEMBER m_calendarSystem NOTIFY calendarSystemChanged)

    /**
     * The available calendar system list
     */
    Q_PROPERTY(QAbstractListModel *calendarSystemModel READ calendarSystemModel CONSTANT)

    /**
     * The index of current choosen calendar system
     */
    Q_PROPERTY(int currentIndex READ currentIndex CONSTANT)

    /**
     * The date offset in days
     */
    Q_PROPERTY(int dateOffset MEMBER m_dateOffset NOTIFY dateOffsetChanged)

public:
    explicit ConfigStorage(QObject *parent = nullptr);

    QAbstractListModel *calendarSystemModel() const;

    int currentIndex() const;

    /**
     * Saves the modifed configuration.
     *
     * @see calendarSystem
     * @see dateOffset
     */
    Q_INVOKABLE void save();

Q_SIGNALS:
    void calendarSystemChanged();
    void dateOffsetChanged();

private:
    KConfigGroup m_generalConfigGroup;

    CalendarSystem::System m_calendarSystem;
    CalendarSystemModel *const m_calendarSystemModel;

    int m_dateOffset; // For the (tabular) Islamic Civil calendar
};

#endif
