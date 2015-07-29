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
#ifndef PLASMA_QUOTA_WATCH_H
#define PLASMA_QUOTA_WATCH_H

#include <QObject>
#include <QList>
#include <QQmlListProperty>

class QTimer;
class QuotaItem;

class QuotaWatch : public QObject
{
    Q_OBJECT

    Q_PROPERTY(bool quotaInstalled READ quotaInstalled)
    Q_PROPERTY(QString status READ status WRITE setQuota NOTIFY statusChanged)
    Q_PROPERTY(QString toolTip READ toolTip WRITE setToolTip NOTIFY toolTipChanged)

    Q_PROPERTY(QQmlListProperty<QuotaItem> quotaItems READ quotaItems NOTIFY quotaItemsChaged)

public:
    QuotaWatch(QObject * parent = nullptr);

    bool quotaInstalled();

    QString status() const;
    QString toolTip() const;

    QQmlListProperty<QuotaItem> quotaItems();
    Q_INVOKABLE int quotaItemCount() const;
    QuotaItem * quotaItem(int index) const;

public Q_SLOTS:
    void setToolTip(const QString & toolTip);
    void setQuota(const QString & quotaString);
    void updateQuota();

Q_SIGNALS:
    void statusChanged();
    void toolTipChanged();
    void quotaItemsChaged();

private:
    QTimer * m_timer;
    QString m_status;
    QString m_toolTip;
    QList<QuotaItem *> m_items;
};

#endif // PLASMA_QUOTA_WATCH_H
