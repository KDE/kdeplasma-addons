/*
 *   SPDX-FileCopyrightText: 2007 Tobias Koenig <tokoe@kde.org>
 *   SPDX-FileCopyrightText: 2008 Marco Martin <notmart@gmail.com>
 *   SPDX-FileCopyrightText: 2008-2011 Matthias Fuchs <mat69@gmx.net>
 *   SPDX-FileCopyrightText: 2012 Reza Fatahilah Shah <rshah0385@kireihana.com>
 *   SPDX-FileCopyrightText: 2015 Marco Martin <mart@kde.org>
 *   SPDX-FileCopyrightText: 2022 Alexander Lohnau <alexander.lohnau@gmx.de>
 *
 *   SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "comic.h"
#include "checknewstrips.h"
#include "comic_debug.h"

#include <QDir>
#include <QScreen>
#include <QWindow>

#include <KLocalizedString>
#include <kconfiggroup.h>

#include "comicmodel.h"
#include "types.h"

ComicApplet::ComicApplet(QObject *parent, const KPluginMetaData &data, const QVariantList &args)
    : Plasma::Applet(parent, data, args)
    , mEngine(new ComicEngine(this))
    , mModel(new ComicModel(mEngine, QStringList(), this))
    , mCheckNewStrips(nullptr)
{
    setHasConfigurationInterface(true);
}

void ComicApplet::init()
{
    configChanged();

    connect(mEngine, &ComicEngine::requestFinished, this, &ComicApplet::dataUpdated);
}

void ComicApplet::dataUpdated(const ComicMetaData &data)
{
    const QString source = data.identifier;
    setBusy(false);

    // disconnect prefetched comic strips
    if (data.reason != RequestReason::View) {
        return;
    }

    mCurrent.setData(data);

    // looking at the last index, thus not mark it as new
    KConfigGroup cg = config();
    if (!data.error && !mCurrent.hasNext() && cg.readEntry(QLatin1String("checkNewComicStripsIntervall"), 30)) {
        setTabHighlighted(mCurrent.id(), false);
    }

    storePosition(mCurrent.hasStored());

    // prefetch the previous and following comic for faster navigation
    if (!data.error && mCurrent.hasNext()) {
        const QString prefetch = mCurrent.id() + QLatin1Char(':') + mCurrent.next();
        mEngine->requestSource(prefetch, RequestReason::Fetch);
    }
    if (!data.error && mCurrent.hasPrev()) {
        const QString prefetch = mCurrent.id() + QLatin1Char(':') + mCurrent.prev();
        mEngine->requestSource(prefetch, RequestReason::Fetch);
    }

    Q_EMIT comicDataChanged();
    Q_EMIT showActualSizeChanged(); // if switching comics the new one might have a different setting
}

void ComicApplet::positionFullView(QWindow *window)
{
    if (!window || !window->screen()) {
        return;
    }

    window->setPosition(window->screen()->availableGeometry().center() - QPoint(window->size().width() / 2, window->size().height() / 2));
}

void ComicApplet::updateUsedComics()
{
    KConfigGroup cg = config();
    const bool enabledProvidersChanged = cg.readEntry("tabIdentifier", QStringList()) != mModel->enabledProviders();
    const int checkInterval = cg.readEntry(QLatin1String("checkNewComicStripsIntervall"), 30);

    if (enabledProvidersChanged) {
        loadProviders();

        KConfigGroup cg = config();
        for (int i = 0; i < mModel->rowCount(); ++i) {
            QModelIndex index = mModel->index(i, 0);
            if (index.data(ComicModel::Roles::ComicEnabledRole).toBool() && checkInterval
                && !cg.readEntry(QLatin1String("lastStripVisited_") + index.data(ComicModel::Roles::ComicPluginRole).toString(), true)) {
                mModel->setHighlight(index, true);
            }
        }
    }

    if (enabledProvidersChanged || !mCheckNewStrips || checkInterval != mCheckNewStrips->minutes()) {
        delete mCheckNewStrips;
        mCheckNewStrips = nullptr;
        if (checkInterval) {
            mCheckNewStrips = new CheckNewStrips(cg.readEntry("tabIdentifier", QStringList()), mEngine, checkInterval, this);
            connect(mCheckNewStrips, &CheckNewStrips::lastStrip, this, &ComicApplet::slotFoundLastStrip);
        }
    }
}

void ComicApplet::tabChanged(const QString &identifier)
{
    if (mCurrent.id() == identifier) {
        return;
    }
    mCurrent = ComicData();
    mCurrent.init(identifier, config());

    KConfigGroup cg = config();
    if (!cg.readEntry("tabIdentifier", QStringList()).isEmpty()) {
        updateComic(mCurrent.stored());
    }
}

void ComicApplet::configChanged()
{
    updateUsedComics();
    mEngine->setMaxComicLimit(config().readEntry("maxComicLimit", 29));
}

void ComicApplet::slotFoundLastStrip(int index, const QString &identifier, const QString &suffix)
{
    Q_UNUSED(index)

    if (mCurrent.id() != identifier) {
        return;
    }

    KConfigGroup cg = config();
    if (suffix != cg.readEntry(QLatin1String("lastStrip_") + identifier, QString())) {
        qCDebug(PLASMA_COMIC) << identifier << "has a newer strip.";
        cg.writeEntry(QLatin1String("lastStripVisited_") + identifier, false);
        updateComic(suffix);
    }
}

void ComicApplet::storePosition(bool store)
{
    mCurrent.storePosition(store);
}

void ComicApplet::updateComic(const QString &identifierSuffix)
{
    const QString id = mCurrent.id();

    if (!id.isEmpty()) {
        setBusy(true);

        const QString identifier = id + QLatin1Char(':') + identifierSuffix;
        mEngine->requestSource(identifier, RequestReason::View);
        setShowActualSize(mCurrent.showActualSize());
    } else {
        setBusy(false);
        qCWarning(PLASMA_COMIC) << "Either no identifier was specified or the engine could not be created:"
                                << "id" << id;
    }
}

// QML

QObject *ComicApplet::availableComicsModel() const
{
    return mModel;
}

ComicData ComicApplet::comicData() const
{
    return mCurrent;
}

bool ComicApplet::showActualSize() const
{
    return mCurrent.showActualSize();
}

void ComicApplet::setShowActualSize(bool show)
{
    if (show == mCurrent.showActualSize()) {
        return;
    }

    mCurrent.setShowActualSize(show);

    Q_EMIT showActualSizeChanged();
}

// Endof QML
void ComicApplet::setTabHighlighted(const QString &id, bool highlight)
{
    QModelIndexList indexList = mModel->match(mModel->index(0, 0), ComicModel::ComicPluginRole, id, 1, Qt::MatchFixedString);
    if (!indexList.empty()) {
        mModel->setHighlight(indexList[0], highlight);
    }
}

void ComicApplet::loadProviders()
{
    mModel->setEnabledProviders(config().readEntry("tabIdentifier", QStringList()));
}

bool ComicApplet::urlExists(const QUrl &url)
{
    QDir dir(url.path());
    return dir.exists();
}

K_PLUGIN_CLASS_WITH_JSON(ComicApplet, "metadata.json")

#include "comic.moc"
