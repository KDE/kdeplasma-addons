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
#include "QuotaItem.h"

#include <QDebug>

QuotaItem::QuotaItem()
    : m_iconName(QStringLiteral("quota"))
    , m_mountPoint()
    , m_usage(0)
    , m_mountString()
    , m_usedString()
{
}

QString QuotaItem::iconName() const
{
    return m_iconName;
}

void QuotaItem::setIconName(const QString &name)
{
    m_iconName = name;
}

QString QuotaItem::mountPoint() const
{
    return m_mountPoint;
}

void QuotaItem::setMountPoint(const QString &mountPoint)
{
    m_mountPoint = mountPoint;
}

int QuotaItem::usage() const
{
    return m_usage;
}

void QuotaItem::setUsage(int usage)
{
    m_usage = usage;
}

QString QuotaItem::mountString() const
{
    return m_mountString;
}

void QuotaItem::setMountString(const QString &mountString)
{
    m_mountString = mountString;
}

QString QuotaItem::usedString() const
{
    return m_usedString;
}

void QuotaItem::setUsedString(const QString &usedString)
{
    m_usedString = usedString;
}

QString QuotaItem::freeString() const
{
    return m_freeString;
}

void QuotaItem::setFreeString(const QString &freeString)
{
    m_freeString = freeString;
}

bool QuotaItem::operator==(const QuotaItem &other) const
{
    return m_mountPoint == other.m_mountPoint
        && m_iconName == other.m_iconName
        && m_usage == other.m_usage
        && m_mountString == other.m_mountString
        && m_usedString == other.m_usedString
        && m_freeString == other.m_freeString;
}

bool QuotaItem::operator!=(const QuotaItem &other) const
{
    return ! (*this == other);
}
