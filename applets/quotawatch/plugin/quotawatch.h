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

/**
 * Class monitoring the file system quota.
 * The monitoring is performed through a timer, running the 'quota'
 * command line tool.
 */
class QuotaWatch : public QObject
{
    Q_OBJECT

    Q_PROPERTY(bool quotaInstalled READ quotaInstalled WRITE setQuotaInstalled NOTIFY quotaInstalledChanged)
    Q_PROPERTY(QString status READ status WRITE setStatus NOTIFY statusChanged)
    Q_PROPERTY(QString toolTip READ toolTip WRITE setToolTip NOTIFY toolTipChanged)
    Q_PROPERTY(QString subToolTip READ subToolTip WRITE setSubToolTip NOTIFY subToolTipChanged)

    Q_PROPERTY(QQmlListProperty<QuotaItem> quotaItems READ quotaItems NOTIFY quotaItemsChaged)

public:
    QuotaWatch(QObject * parent = nullptr);

    /**
     * Checks whether 'quota' is installed or not.
     */
    bool quotaInstalled() const;

    /**
     * Returns the current status of the notification. Valid notifications
     * are, with increasing priority:
     * - "status-not-installed": quota is not installed
     * - "status-ok": quota is still ok (enough spaces free)
     * - "status-80": geting closer to quota limit (>= 80% used)
     * - "status-90": geting closer to quota limit (>= 90% used)
     * - "status-98": clost to critical limit (>= 98% used)
     */
    QString status() const;
    QString toolTip() const;
    QString subToolTip() const;

    QQmlListProperty<QuotaItem> quotaItems();
    Q_INVOKABLE int quotaItemCount() const;
    QuotaItem * quotaItem(int index) const;

public Q_SLOTS:
    void setQuotaInstalled(bool installed);
    void setStatus(const QString & status);
    void setToolTip(const QString & toolTip);
    void setSubToolTip(const QString & subToolTip);
    void updateQuota();

Q_SIGNALS:
    void quotaInstalledChanged();
    void statusChanged();
    void toolTipChanged();
    void subToolTipChanged();
    void quotaItemsChaged();

private:
    QTimer * m_timer;
    bool m_quotaInstalled;
    QString m_status;
    QString m_toolTip;
    QString m_subToolTip;
    QList<QuotaItem *> m_items;
};

#endif // PLASMA_QUOTA_WATCH_H
