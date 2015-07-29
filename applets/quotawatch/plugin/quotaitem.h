/*
 * Copyright (C) 2015 Dominik Haumann <dhaumann@kde.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */
#ifndef PLASMA_QUOTA_ITEM_H
#define PLASMA_QUOTA_ITEM_H

#include <QObject>
#include <QString>

class QuotaItem : public QObject
{
    Q_OBJECT

    // needed in QML
    Q_PROPERTY(QString iconName READ iconName WRITE setIconName NOTIFY iconNameChanged)
    Q_PROPERTY(QString mountString READ mountString WRITE setMountString NOTIFY mountStringChanged)
    Q_PROPERTY(QString detailString READ detailString WRITE setDetailString NOTIFY detailStringChanged)

    Q_PROPERTY(QString mountPoint READ mountPoint WRITE setMountPoint NOTIFY mountPointChanged)
    Q_PROPERTY(qreal usage READ usage WRITE setUsage NOTIFY usageChanged)

public:
    explicit QuotaItem(QObject * parent = nullptr);

public Q_SLOTS:
    QString mountPoint() const;
    void setMountPoint(const QString & mountPoint);

    qreal usage() const;
    void setUsage(qreal usage);

public Q_SLOTS:
    QString iconName() const;
    void setIconName(const QString & name);

    QString mountString() const;
    void setMountString(const QString & mountString);

    QString detailString() const;
    void setDetailString(const QString & detailString);

Q_SIGNALS:
    void mountPointChanged();
    void usageChanged();
    void iconNameChanged();
    void mountStringChanged();
    void detailStringChanged();

private:
    QString m_iconName;
    QString m_mountPoint;
    qreal m_usage;
    QString m_mountString;
    QString m_detailString;
};

#endif // PLASMA_QUOTA_ITEM_H
