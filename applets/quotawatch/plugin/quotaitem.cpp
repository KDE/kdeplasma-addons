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
#include "quotaitem.h"

#include <KLocalizedString>
#include <KFormat>

#include <QTimer>
#include <QProcess>
#include <QRegularExpression>
#include <QStandardPaths>
#include <QDebug>

QuotaItem::QuotaItem()
    : QObject()
{
}

QString QuotaItem::mountPoint() const
{
    return m_mountPoint;
}

void QuotaItem::setMountPoint(const QString & mountPoint)
{
    if (m_mountPoint != mountPoint) {
        m_mountPoint = mountPoint;
        emit mountPointChanged();
    }
}

qreal QuotaItem::usage() const
{
    return m_usage;
}

void QuotaItem::setUsage(qreal usage)
{
    if (m_usage != usage) {
        m_usage = usage;
        emit usageChanged();
    }
}

QString QuotaItem::icon() const
{
    return m_icon;
}

void QuotaItem::setIcon(const QString & icon)
{
    if (m_icon != icon) {
        m_icon = icon;
        emit iconChanged();
    }
}

QString QuotaItem::mountString() const
{
    return m_mountString;
}

void QuotaItem::setMountString(const QString & mountString)
{
    if (m_mountString != mountString) {
        m_mountString = mountString;
        emit mountStringChanged();
    }
}

QString QuotaItem::usageString() const
{
    return m_usageString;
}

void QuotaItem::setUsageString(const QString & usageString)
{
    if (m_usageString != usageString) {
        m_usageString = usageString;
        emit usageStringChanged();
    }
}
