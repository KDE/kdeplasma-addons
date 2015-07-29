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
#include "quotawatch.h"
#include "quotaitem.h"

#include <KLocalizedString>
#include <KFormat>

#include <QTimer>
#include <QProcess>
#include <QRegularExpression>
#include <QStandardPaths>
#include <QDebug>

QuotaWatch::QuotaWatch(QObject * parent)
    : QObject(parent)
    , m_timer(new QTimer(this))
{
    connect(m_timer, &QTimer::timeout, this, &QuotaWatch::updateQuota);
    m_timer->start(5 * 1000);
    updateQuota();
}

QQmlListProperty<QuotaItem> QuotaWatch::quotaItems()
{
    return QQmlListProperty<QuotaItem>(this, m_items);
}

int QuotaWatch::quotaItemCount() const
{
    return m_items.count();
}

QuotaItem * QuotaWatch::quotaItem(int index) const
{
    if (index < 0 || index >= m_items.count()) {
        return nullptr;
    }

    return m_items.at(index);
}

bool QuotaWatch::quotaInstalled()
{
    return ! QStandardPaths::findExecutable(QStringLiteral("quota")).isEmpty();
}

QString QuotaWatch::status() const
{
    return m_status;
}

QString QuotaWatch::toolTip() const
{
    return m_toolTip;
}

void QuotaWatch::setToolTip(const QString & toolTip)
{
    if (m_toolTip != toolTip) {
        m_toolTip = toolTip;
        emit toolTipChanged();
    }    
}

void QuotaWatch::setQuota(const QString & quotaString)
{
    if (m_status != quotaString) {
        m_status = quotaString;
        emit statusChanged();
    }
}

static bool isQuotaLine(const QString & line)
{
    const int iMax = line.size();
    for (int i = 0; i < iMax; ++i) {
        if (!line[i].isSpace() && line[i] == QLatin1Char('/')) {
            return true;
        }
    }
    return false;
}

static bool runQuotaApp(QString & stdout)
{
    // Try to run 'quota'
    const QStringList args = QStringList()
        << QStringLiteral("--show-mntpoint")      // second entry is e.g. '/home'
        << QStringLiteral("--hide-device")        // hide e.g. /dev/sda3
        << QStringLiteral("--no-mixed-pathnames") // trim leading slashes from NFSv4 mountpoints
        << QStringLiteral("--all-nfs")            // show all mount points
        << QStringLiteral("--no-wrap")            // do not wrap long lines
        << QStringLiteral("--quiet-refuse");      // no not print error message when NFS server does not respond

    QProcess proc;
    proc.start(QStringLiteral("quota"), args);

    const bool success = proc.waitForStarted() && proc.waitForFinished();
    if (!success) {
        return false;
    }

    // get stdout
    proc.closeWriteChannel();
    stdout = QString::fromLocal8Bit(proc.readAllStandardOutput());

    return true;
}

void QuotaWatch::updateQuota()
{
    for (auto item : m_items) {
        item->deleteLater();
    }
    m_items.clear();
    emit quotaItemsChaged();

    if (!quotaInstalled()) {
        setQuota(i18n("Quota missing"));
        return;
    }

    // get quota output
    QString rawData;
    const bool success = runQuotaApp(rawData);
    if (!success) {
        setQuota(i18n("Running quota failed"));
        return;
    }

    QStringList lines = rawData.split(QRegularExpression(QStringLiteral("[\r\n]")), QString::SkipEmptyParts);
    lines += lines;
    lines += lines;

    // Example output: `quota`
    // Disk quotas for user dh (uid 1000):
    //      Filesystem   blocks  quota    limit     grace   files    quota   limit   grace
    //       /home     16296500  50000000 60000000          389155       0       0
    // 123456          ^......we want these......^

    QStringList quotas;
    // assumption: Filesystem starts with slash
    for (const QString & line : lines) {
        qDebug() << line << isQuotaLine(line);
        if (!isQuotaLine(line)) {
            continue;
        }

        const QStringList parts = line.split(QLatin1Char(' '), QString::SkipEmptyParts);
        // valid lines range from 7 to 9 parts (grace not always there):
        //      Filesystem   blocks  quota    limit     grace   files    quota   limit   grace
        //       /home     16296500  50000000 60000000          389155       0       0
        //       /home     16296500  50000000 60000000      6   389155       0       0
        //       /home     16296500  50000000 60000000      4   389155       0       0       5

        if (parts.size() < 4) {
            continue;
        }

        // 'quota' uses kilo bytes -> factor 1024
        // NOTE: int is not large enough, hence qint64
        const qint64 used = parts[1].toLongLong() * 1024;
        const qint64 softLimit = parts[2].toLongLong() * 1024;
        const qreal percent = used * 100.0 / softLimit;

        KFormat fmt;
        QString quotaLine = i18n("%1: %2% (%3 out of %4 used)",
                                 parts[0],
                                 qRound(percent),
                                 fmt.formatByteSize(used),
                                 fmt.formatByteSize(softLimit));
        quotas.append(quotaLine);

        auto item = new QuotaItem();
        item->setIconName(QStringLiteral("network-server-database"));
        item->setMountPoint(parts[0]);
        item->setUsage(percent);
        item->setMountString(i18nc("usage of quota, e.g.: '/home/bla: 38\%'", "%1: %2%", parts[0], qRound(percent)));
        item->setDetailString(i18nc("e.g.: 12 GiB of 20 GiB used", "%1 of %2 used", fmt.formatByteSize(used), fmt.formatByteSize(softLimit)));

        m_items.append(item);
        emit quotaItemsChaged();
    }

//     qDebug() << "QUOTAS:" << quotas;
    if (!quotas.isEmpty()) {
        setQuota(i18n("Quota OK"));
        setToolTip(quotas.join(QLatin1Char('\n')));
    } else {
        setQuota(i18n("Quota OK"));
        setToolTip(i18n("No quota restrictions found."));
    }
}
