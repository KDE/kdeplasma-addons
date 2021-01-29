/*
 *   SPDX-FileCopyrightText: 2007 Tobias Koenig <tokoe@kde.org>
 *   SPDX-FileCopyrightText: 2008-2010 Matthias Fuchs <mat69@gmx.net>
 *
 *   SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "comicupdater.h"
#include "comicmodel.h"

#include <QSortFilterProxyModel>
#include <QTimer>

#include <KConfigDialog>
#include <KNS3/DownloadDialog>
#include <KNSCore/Engine>

ComicUpdater::ComicUpdater(QObject *parent)
    : QObject(parent)
    , mEngine(nullptr)
    , mUpdateIntervall(3)
    , m_updateTimer(nullptr)
    , mProvidersLoaded(false)
{
}

ComicUpdater::~ComicUpdater()
{
}

void ComicUpdater::init(const KConfigGroup &group)
{
    mGroup = group;
}

void ComicUpdater::load()
{
    // check when the last update happened and update if necessary
    mUpdateIntervall = mGroup.readEntry("updateInterval", 3);
    if (mUpdateIntervall) {
        mLastUpdate = mGroup.readEntry("lastUpdate", QDateTime());
        checkForUpdate();
    }
}

void ComicUpdater::save()
{
    mGroup.writeEntry("updateInterval", mUpdateIntervall);
}

void ComicUpdater::setInterval(int interval)
{
    mUpdateIntervall = interval;
}

int ComicUpdater::interval() const
{
    return mUpdateIntervall;
}

void ComicUpdater::checkForUpdate()
{
    // start a timer to check each hour, if KNS3 should look for updates
    if (!m_updateTimer) {
        m_updateTimer = new QTimer(this);
        connect(m_updateTimer, &QTimer::timeout, this, &ComicUpdater::checkForUpdate);
        m_updateTimer->start(1 * 60 * 60 * 1000);
    }

    if (!mLastUpdate.isValid() || (mLastUpdate.addDays(mUpdateIntervall) < QDateTime::currentDateTime())) {
        mLastUpdate = QDateTime::currentDateTime();
        mGroup.writeEntry("lastUpdate", mLastUpdate);
        if (mProvidersLoaded) {
            engine()->checkForUpdates();
        } else {
            connect(engine(), &KNSCore::Engine::signalProvidersLoaded, this, [this]() {
                engine()->checkForUpdates();
            });
        }
    }
}

void ComicUpdater::slotUpdatesFound(const KNSCore::EntryInternal::List &entries)
{
    for (int i = 0; i < entries.count(); ++i) {
        engine()->install(entries[i]);
    }
}

KNSCore::Engine *ComicUpdater::engine()
{
    if (!mEngine) {
        mEngine = new KNSCore::Engine(this);
        if (mEngine->init(QStringLiteral("comic.knsrc"))) {
            connect(mEngine, &KNSCore::Engine::signalUpdateableEntriesLoaded, this, &ComicUpdater::slotUpdatesFound);
            connect(mEngine, &KNSCore::Engine::signalProvidersLoaded, this, [this]() {
                mProvidersLoaded = true;
            });
        }
    }
    return mEngine;
}
