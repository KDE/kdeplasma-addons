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

#include <QString>
#include <QMetaType>

/**
 * Class that holds all quota info for one mount point.
 */
class QuotaItem
{
public:
    QuotaItem();

    QString mountPoint() const;
    void setMountPoint(const QString &mountPoint);

    int usage() const;
    void setUsage(int usage);

    QString iconName() const;
    void setIconName(const QString &name);

    QString mountString() const;
    void setMountString(const QString &mountString);

    QString usedString() const;
    void setUsedString(const QString &usedString);

    QString freeString() const;
    void setFreeString(const QString &freeString);

    bool operator==(const QuotaItem &other) const;
    bool operator!=(const QuotaItem &other) const;

private:
    QString m_iconName;
    QString m_mountPoint;
    int m_usage;
    QString m_mountString;
    QString m_usedString;
    QString m_freeString;
};

Q_DECLARE_METATYPE(QuotaItem)

#endif // PLASMA_QUOTA_ITEM_H
