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

class QTimer;
class QuotaListModel;

/**
 * Class monitoring the file system quota.
 * The monitoring is performed through a timer, running the 'quota'
 * command line tool.
 */
class DiskQuota : public QObject
{
    Q_OBJECT

    Q_PROPERTY(bool quotaInstalled READ quotaInstalled WRITE setQuotaInstalled NOTIFY quotaInstalledChanged)
    Q_PROPERTY(QString status READ status WRITE setStatus NOTIFY statusChanged)
    Q_PROPERTY(QString toolTip READ toolTip WRITE setToolTip NOTIFY toolTipChanged)
    Q_PROPERTY(QString subToolTip READ subToolTip WRITE setSubToolTip NOTIFY subToolTipChanged)

public:
    DiskQuota(QObject * parent = nullptr);

    /**
     * Checks whether 'quota' is installed or not.
     */
    bool quotaInstalled() const;

    /**
     * Returns the current status of the notification. Valid notifications
     * are, with increasing priority:
     * - "status-not-installed": quota is not installed
     * - "status-ok": quota is still ok (enough spaces free)
     * - "status-75": geting closer to quota limit (>= 75% used)
     * - "status-90": geting closer to quota limit (>= 90% used)
     * - "status-critical": close to critical limit (>= 98% used)
     */
    QString status() const;
    QString toolTip() const;
    QString subToolTip() const;


public Q_SLOTS:
    void setQuotaInstalled(bool installed);
    void setStatus(const QString & status);
    void setToolTip(const QString & toolTip);
    void setSubToolTip(const QString & subToolTip);
    void updateQuota();
    QuotaListModel * model() const;

Q_SIGNALS:
    void quotaInstalledChanged();
    void statusChanged();
    void toolTipChanged();
    void subToolTipChanged();

private:
    QTimer * m_timer;
    bool m_quotaInstalled;
    QString m_status;
    QString m_toolTip;
    QString m_subToolTip;
    QuotaListModel * m_model;
};

#endif // PLASMA_QUOTA_WATCH_H
