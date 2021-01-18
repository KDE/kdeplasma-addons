/*
 *   SPDX-FileCopyrightText: 2007 Tobias Koenig <tokoe@kde.org>
 *   SPDX-FileCopyrightText: 2008-2010 Matthias Fuchs <mat69@gmx.net>
 *
 *   SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef COMICUPDATER_H
#define COMICUPDATER_H

#include <QTime>
#include <QWidget>

#include <KNSCore/EntryInternal>
#include <Plasma/DataEngine>

namespace KNS3
{
}
namespace KNSCore
{
class DownloadManager;
}

namespace Plasma
{
class DataEngine;
}

class ComicUpdater : public QObject
{
    Q_OBJECT

public:
    explicit ComicUpdater(QObject *parent = nullptr);
    ~ComicUpdater() override;

    void init(const KConfigGroup &group);

    void load();
    void save();
    void setInterval(int interval);
    int interval() const;

private Q_SLOTS:
    /**
     * Will check if an update is needed, if so will search
     * for updates and do them automatically
     */
    void checkForUpdate();
    void slotUpdatesFound(const KNSCore::EntryInternal::List &entries);

private:
    KNSCore::DownloadManager *downloadManager();

private:
    KNSCore::DownloadManager *mDownloadManager;
    KConfigGroup mGroup;
    int mUpdateIntervall;
    QDateTime mLastUpdate;
    QTimer *m_updateTimer;
};

#endif
